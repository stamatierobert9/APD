import java.io.*;
import java.util.*;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;

public class Tema1 {
    public static void main(String[] args) throws IOException, InterruptedException {
        if (args.length < 3) {
            System.out.println("Usage: java Tema1 <threads> <articles_file> <aux_file>");
            return;
        }

        int numThreads = Integer.parseInt(args[0]);
        String articlesFile = args[1];
        String auxFile = args[2];

        GlobalState state = new GlobalState();

        // 1. Citire fișiere input
        readArticleList(articlesFile, state);
        readAuxiliaryFiles(auxFile, state);

        // 2. Inițializare Thread-uri și Barieră
        // Bariera are numThreads + 1 (Workerii + Main-ul care așteaptă la final)
        CyclicBarrier barrier = new CyclicBarrier(numThreads + 1);
        WorkerThread[] workers = new WorkerThread[numThreads];

        for (int i = 0; i < numThreads; i++) {
            workers[i] = new WorkerThread(i, numThreads, state, barrier);
            workers[i].start();
        }

        // 3. Așteptăm ca workerii să termine toate etapele (4 apeluri de await în worker)
        try {
            barrier.await(); // Wait for read
            barrier.await(); // Wait for count
            barrier.await(); // Wait for filter
            barrier.await(); // Wait for keywords
        } catch (Exception e) {
            e.printStackTrace();
        }

        // 4. Generare Output-uri (Sequential, pe main thread, pentru determinism)
        // Sortăm articolele curățate cronologic [cite: 73]
        Collections.sort(state.cleanArticles);

        generateAllArticlesFile(state.cleanArticles);
        generateCategoryFiles(state);
        generateLanguageFiles(state);
        generateKeywordsFile(state);
        generateReports(state);
    }

    // --- Helper Methods for IO ---

    private static void readArticleList(String path, GlobalState state) throws IOException {
        try (BufferedReader br = new BufferedReader(new FileReader(path))) {
            int n = Integer.parseInt(br.readLine().trim());
            for (int i = 0; i < n; i++) {
                state.inputFiles.add(br.readLine().trim());
            }
        }
    }

    private static void readAuxiliaryFiles(String path, GlobalState state) throws IOException {
        String langPath, catPath, exclPath;
        File baseDir = new File(path).getParentFile();

        try (BufferedReader br = new BufferedReader(new FileReader(path))) {
            br.readLine(); // skip count
            langPath = resolvePath(baseDir, br.readLine().trim());
            catPath = resolvePath(baseDir, br.readLine().trim());
            exclPath = resolvePath(baseDir, br.readLine().trim());
        }

        // Read Languages
        try (BufferedReader br = new BufferedReader(new FileReader(langPath))) {
            br.readLine(); // skip count
            String line;
            while ((line = br.readLine()) != null) state.targetLanguages.add(line.trim());
        }

        // Read Categories
        try (BufferedReader br = new BufferedReader(new FileReader(catPath))) {
            br.readLine(); // skip count
            String line;
            while ((line = br.readLine()) != null) state.targetCategories.add(line.trim());
        }

        // Read Stop words
        try (BufferedReader br = new BufferedReader(new FileReader(exclPath))) {
            br.readLine(); // skip count
            String line;
            while ((line = br.readLine()) != null) state.excludeWords.add(line.trim());
        }
    }

    private static String resolvePath(File dir, String path) {
        if (path.startsWith("./")) {
            return new File(dir, path.substring(2)).getAbsolutePath();
        }
        return new File(dir, path).getAbsolutePath();
    }

    private static void generateAllArticlesFile(List<Article> articles) throws IOException {
        try (PrintWriter pw = new PrintWriter("all_articles.txt")) {
            for (Article a : articles) {
                pw.println(a.uuid + " " + a.published);
            }
        }
    }

    private static void generateCategoryFiles(GlobalState state) throws IOException {
        for (String cat : state.targetCategories) {
            List<String> uuids = new ArrayList<>();
            for (Article a : state.cleanArticles) {
                if (a.categories.contains(cat)) {
                    uuids.add(a.uuid);
                }
            }

            if (!uuids.isEmpty()) {
                Collections.sort(uuids); // Lexicografic [cite: 63]
                String filename = cat.replace(",", "").replace(" ", "_") + ".txt"; // Normalizare [cite: 61]
                try (PrintWriter pw = new PrintWriter(filename)) {
                    for (String id : uuids) pw.println(id);
                }
            }
        }
    }

    private static void generateLanguageFiles(GlobalState state) throws IOException {
        for (String lang : state.targetLanguages) {
            List<String> uuids = new ArrayList<>();
            for (Article a : state.cleanArticles) {
                if (lang.equals(a.language)) {
                    uuids.add(a.uuid);
                }
            }

            if (!uuids.isEmpty()) {
                Collections.sort(uuids);
                try (PrintWriter pw = new PrintWriter(lang + ".txt")) {
                    for (String id : uuids) pw.println(id);
                }
            }
        }
    }

    private static void generateKeywordsFile(GlobalState state) throws IOException {
        List<Map.Entry<String, AtomicInteger>> list = new ArrayList<>(state.keywordCounts.entrySet());

        // Sortare: Descrescător după count, apoi lexicografic după cuvânt [cite: 93]
        list.sort((e1, e2) -> {
            int c = Integer.compare(e2.getValue().get(), e1.getValue().get());
            if (c != 0) return c;
            return e1.getKey().compareTo(e2.getKey());
        });

        try (PrintWriter pw = new PrintWriter("keywords_count.txt")) {
            for (Map.Entry<String, AtomicInteger> e : list) {
                pw.println(e.getKey() + " " + e.getValue().get());
            }
        }
    }

    private static void generateReports(GlobalState state) throws IOException {
        try (PrintWriter pw = new PrintWriter("reports.txt")) {
            // 1. Duplicates found (Total raw - Total unique)
            // Atenție: cerința zice "Numărul total de articole eliminate"
            // Deoarece eliminăm AMBELE copii, numărul eliminat este size() inițial - size() final.
            int dropped = state.allRawArticles.size() - state.cleanArticles.size();
            pw.println("duplicates_found " + dropped);

            // 2. Unique articles
            pw.println("unique_articles " + state.cleanArticles.size());

            // 3. Best Author (Calculam pe loc)
            Map<String, Integer> authCounts = new HashMap<>();
            for (Article a : state.cleanArticles) authCounts.merge(a.author, 1, Integer::sum);
            String bestAuth = authCounts.entrySet().stream()
                    .sorted((e1, e2) -> {
                        int c = e2.getValue().compareTo(e1.getValue());
                        return c != 0 ? c : e1.getKey().compareTo(e2.getKey());
                    }).map(e -> e.getKey() + " " + e.getValue()).findFirst().orElse("");
            pw.println("best_author " + bestAuth); // Formatul poate varia, verifică exemplul exact

            // 4. Top Language
            Map<String, Integer> langCounts = new HashMap<>();
            for (Article a : state.cleanArticles) langCounts.merge(a.language, 1, Integer::sum);
            String topLang = langCounts.entrySet().stream()
                    .sorted((e1, e2) -> {
                        int c = e2.getValue().compareTo(e1.getValue());
                        return c != 0 ? c : e1.getKey().compareTo(e2.getKey());
                    }).map(e -> e.getKey() + " " + e.getValue()).findFirst().orElse("");
            pw.println("top_language " + topLang);

            // 5. Top Category
            Map<String, Integer> catCounts = new HashMap<>();
            for (Article a : state.cleanArticles) {
                for (String c : a.categories) {
                    if (state.targetCategories.contains(c)) {
                        // Normalizare nume categorie pentru statistici?
                        // Cerința zice: top_category <categorie_normalizata> <count> [cite: 115]
                        String norm = c.replace(",", "").replace(" ", "_");
                        catCounts.merge(norm, 1, Integer::sum);
                    }
                }
            }
            String topCat = catCounts.entrySet().stream()
                    .sorted((e1, e2) -> {
                        int c = e2.getValue().compareTo(e1.getValue());
                        return c != 0 ? c : e1.getKey().compareTo(e2.getKey());
                    }).map(e -> e.getKey() + " " + e.getValue()).findFirst().orElse("");
            pw.println("top_category " + topCat);

            // 6. Most recent article
            // Lista e deja sortată cronologic descrescător. Primul e cel mai recent.
            if (!state.cleanArticles.isEmpty()) {
                Article mostRecent = state.cleanArticles.get(0);
                pw.println("most_recent_article " + mostRecent.published + " " + mostRecent.url);
            }

            // 7. Top Keyword En
            List<Map.Entry<String, AtomicInteger>> kList = new ArrayList<>(state.keywordCounts.entrySet());
            kList.sort((e1, e2) -> {
                int c = Integer.compare(e2.getValue().get(), e1.getValue().get());
                return c != 0 ? c : e1.getKey().compareTo(e2.getKey());
            });
            if (!kList.isEmpty()) {
                pw.println("top_keyword_en " + kList.get(0).getKey() + " " + kList.get(0).getValue());
            }
        }
    }
}