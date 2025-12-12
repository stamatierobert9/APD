import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;

public class WorkerThread extends Thread {
    private final int id;
    private final int totalThreads;
    private final GlobalState state;
    private final CyclicBarrier barrier;

    public WorkerThread(int id, int totalThreads, GlobalState state, CyclicBarrier barrier) {
        this.id = id;
        this.totalThreads = totalThreads;
        this.state = state;
        this.barrier = barrier;
    }

    @Override
    public void run() {
        try {
            // --- ETAPA 1: Citire și Parsare ---
            // Împărțim fișierele de intrare între thread-uri (round-robin)
            for (int i = id; i < state.inputFiles.size(); i += totalThreads) {
                String filePath = state.inputFiles.get(i);
                try {
                    List<Article> articles = SimpleJsonParser.parseFile(filePath);
                    state.allRawArticles.addAll(articles);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

            // Așteptăm ca toate thread-urile să termine citirea
            barrier.await();

            // --- ETAPA 2: Numărare pentru Duplicate ---
            // Fiecare thread ia o porțiune din articolele brute și numără UUID și Titluri
            int totalRaw = state.allRawArticles.size();
            for (int i = id; i < totalRaw; i += totalThreads) {
                Article art = state.allRawArticles.get(i);
                state.uuidCounts.merge(art.uuid, 1, Integer::sum);
                state.titleCounts.merge(art.title, 1, Integer::sum);
            }

            barrier.await();

            // --- ETAPA 3: Filtrare Duplicate ---
            // Regulă: Se elimină AMBELE dacă există duplicate [cite: 54, 316]
            for (int i = id; i < totalRaw; i += totalThreads) {
                Article art = state.allRawArticles.get(i);
                int uCount = state.uuidCounts.get(art.uuid);
                int tCount = state.titleCounts.get(art.title);

                // Păstrăm doar dacă apare o singură dată și uuid-ul și titlul
                if (uCount == 1 && tCount == 1) {
                    state.cleanArticles.add(art);
                }
            }

            barrier.await();

            // --- ETAPA 4: Procesare Statistici (Keywords) ---
            // Procesăm doar articolele curate
            int totalClean = state.cleanArticles.size();
            for (int i = id; i < totalClean; i += totalThreads) {
                Article art = state.cleanArticles.get(i);

                // Procesare Keywords doar pentru articole în engleză [cite: 89]
                if ("english".equals(art.language)) {
                    processKeywords(art.text);
                }
            }

            // Barieră finală înainte ca main-ul să scrie fișierele
            barrier.await();

        } catch (InterruptedException | BrokenBarrierException e) {
            e.printStackTrace();
        }
    }

    private void processKeywords(String text) {
        if (text == null) return;

        // Normalizare text [cite: 96, 97, 98]
        // 1. Convert to lower case
        String lower = text.toLowerCase();

        // 2. Split by space
        String[] words = lower.split("\\s+");

        // Folosim un Set local pentru a număra în câte articole DISTINCTE apare cuvântul [cite: 92]
        List<String> uniqueWordsInThisArticle = new ArrayList<>();

        for (String w : words) {
            // 3. Remove non-letters
            String cleanWord = w.replaceAll("[^a-z]", "");

            if (!cleanWord.isEmpty() && !state.excludeWords.contains(cleanWord)) {
                if (!uniqueWordsInThisArticle.contains(cleanWord)) {
                    uniqueWordsInThisArticle.add(cleanWord);
                }
            }
        }

        // Adăugăm la contorul global
        for (String w : uniqueWordsInThisArticle) {
            state.addKeyword(w);
        }
    }
}