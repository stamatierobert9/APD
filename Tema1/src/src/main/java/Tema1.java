import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Tema1 {
    // Structuri globale thread-safe
    private static final List<Article> validArticles = Collections.synchronizedList(new ArrayList<>());
    private static final Set<String> seenUuids = ConcurrentHashMap.newKeySet();
    private static final Set<String> seenTitles = ConcurrentHashMap.newKeySet();

    // Countere atomice
    private static final AtomicInteger duplicatesFound = new AtomicInteger(0);

    // Date de configurare
    private static Set<String> targetLanguages = new HashSet<>();
    private static Set<String> targetCategories = new HashSet<>();
    private static Set<String> englishStopWords = new HashSet<>();

    private static final Object deduplicationLock = new Object();

    public static void main(String[] args) throws InterruptedException, IOException {
        if (args.length < 3) {
            System.err.println("Usage: java Tema1 <threads> <articles_file> <aux_file>");
            return;
        }

        int numThreads = Integer.parseInt(args[0]);
        String articlesListFile = args[1];
        String auxListFile = args[2];

        readAuxiliaryFiles(auxListFile);

        Queue<String> fileQueue = new ConcurrentLinkedQueue<>();
        try (BufferedReader br = new BufferedReader(new FileReader(articlesListFile))) {
            String line = br.readLine();
            while ((line = br.readLine()) != null) {
                if (!line.trim().isEmpty()) {
                    fileQueue.add(line.trim());
                }
            }
        }

        Thread[] workers = new Thread[numThreads];
        for (int i = 0; i < numThreads; i++) {
            workers[i] = new Thread(new Worker(fileQueue));
            workers[i].start();
        }

        for (int i = 0; i < numThreads; i++) {
            workers[i].join();
        }

        generateOutputs();
    }

    static class Worker implements Runnable {
        private final Queue<String> fileQueue;
        private final ObjectMapper mapper = new ObjectMapper();

        public Worker(Queue<String> fileQueue) {
            this.fileQueue = fileQueue;
        }

        @Override
        public void run() {
            String filePath;
            while ((filePath = fileQueue.poll()) != null) {
                processFile(filePath);
            }
        }

        private void processFile(String filePath) {
            File file = new File(filePath);
            if (!file.exists()) return;

            try {
                // Citire fisier JSON
                List<Article> articles = mapper.readValue(file, new TypeReference<List<Article>>() {});

                for (Article article : articles) {
                    if (article.uuid == null || article.title == null) continue;

                    boolean isDuplicate = false;
                    // Bloc sincronizat pentru verificare atomica si adaugare
                    synchronized (deduplicationLock) {
                        if (seenUuids.contains(article.uuid) || seenTitles.contains(article.title)) {
                            isDuplicate = true;
                            duplicatesFound.incrementAndGet();
                        } else {
                            seenUuids.add(article.uuid);
                            seenTitles.add(article.title);
                            validArticles.add(article);
                        }
                    }
                }
            } catch (IOException e) {
                // e.printStackTrace(); // Decommentat pentru debugging daca e cazul
            }
        }
    }

    private static void readAuxiliaryFiles(String auxListFile) throws IOException {
        BufferedReader br = new BufferedReader(new FileReader(auxListFile));
        br.readLine(); // Skip count

        String langPath = br.readLine();
        String catPath = br.readLine();
        String wordsPath = br.readLine();
        br.close();

        targetLanguages = readSetFromFile(langPath);
        targetCategories = readSetFromFile(catPath);
        englishStopWords = readSetFromFile(wordsPath);
    }

    private static Set<String> readSetFromFile(String path) throws IOException {
        Set<String> set = new HashSet<>();
        File f = new File(path);
        if (!f.exists()) return set;

        try (BufferedReader br = new BufferedReader(new FileReader(f))) {
            br.readLine();
            String line;
            while ((line = br.readLine()) != null) {
                if (!line.trim().isEmpty()) {
                    set.add(line.trim());
                }
            }
        }
        return set;
    }

    private static void generateOutputs() throws IOException {
        // Sortare: Data Descrescator, UUID Crescator
        validArticles.sort((a1, a2) -> {
            int dateComp = a2.published.compareTo(a1.published);
            if (dateComp != 0) return dateComp;
            return a1.uuid.compareTo(a2.uuid);
        });

        try (PrintWriter pw = new PrintWriter("all_articles.txt")) {
            for (Article a : validArticles) {
                pw.println(a.uuid + " " + a.published);
            }
        }

        Map<String, List<String>> catMap = new HashMap<>();
        Map<String, List<String>> langMap = new HashMap<>();

        Map<String, Integer> authorCounts = new HashMap<>();
        Map<String, Integer> langCounts = new HashMap<>();
        Map<String, Integer> catCounts = new HashMap<>();
        Map<String, Integer> keywordCounts = new HashMap<>();

        for (Article a : validArticles) {
            if (a.categories != null) {
                for (String cat : a.categories) {
                    if (targetCategories.contains(cat)) {
                        String normCat = cat.replace(" ", "_").replace(",", "");
                        catMap.computeIfAbsent(normCat, k -> new ArrayList<>()).add(a.uuid);
                        catCounts.put(cat, catCounts.getOrDefault(cat, 0) + 1);
                    }
                }
            }

            if (a.language != null && targetLanguages.contains(a.language)) {
                langMap.computeIfAbsent(a.language, k -> new ArrayList<>()).add(a.uuid);
                langCounts.put(a.language, langCounts.getOrDefault(a.language, 0) + 1);
            }

            if (a.author != null) {
                authorCounts.put(a.author, authorCounts.getOrDefault(a.author, 0) + 1);
            }

            if ("english".equals(a.language) && a.text != null) {
                processKeywords(a.text, keywordCounts);
            }
        }

        for (Map.Entry<String, List<String>> entry : catMap.entrySet()) {
            writeSortedIds(entry.getKey() + ".txt", entry.getValue());
        }

        for (Map.Entry<String, List<String>> entry : langMap.entrySet()) {
            writeSortedIds(entry.getKey() + ".txt", entry.getValue());
        }

        writeKeywordsCount(keywordCounts);
        writeReports(authorCounts, langCounts, catCounts, keywordCounts);
    }

    private static void processKeywords(String text, Map<String, Integer> keywordCounts) {
        Set<String> uniqueWordsInArticle = new HashSet<>();
        // Split text, remove chars non-alpha
        String[] tokens = text.toLowerCase().split(" ");
        for (String token : tokens) {
            String clean = token.replaceAll("[^a-z]", "");
            if (!clean.isEmpty() && !englishStopWords.contains(clean)) {
                uniqueWordsInArticle.add(clean);
            }
        }

        for (String word : uniqueWordsInArticle) {
            keywordCounts.put(word, keywordCounts.getOrDefault(word, 0) + 1);
        }
    }

    private static void writeSortedIds(String filename, List<String> uuids) throws IOException {
        Collections.sort(uuids);
        try (PrintWriter pw = new PrintWriter(filename)) {
            for (String id : uuids) {
                pw.println(id);
            }
        }
    }

    private static void writeKeywordsCount(Map<String, Integer> counts) throws IOException {
        List<Map.Entry<String, Integer>> sorted = new ArrayList<>(counts.entrySet());
        sorted.sort((e1, e2) -> {
            int valComp = e2.getValue().compareTo(e1.getValue());
            if (valComp != 0) return valComp;
            return e1.getKey().compareTo(e2.getKey());
        });

        try (PrintWriter pw = new PrintWriter("keywords_count.txt")) {
            for (Map.Entry<String, Integer> entry : sorted) {
                pw.println(entry.getKey() + " " + entry.getValue());
            }
        }
    }

    private static void writeReports(
            Map<String, Integer> authorCounts,
            Map<String, Integer> langCounts,
            Map<String, Integer> catCounts,
            Map<String, Integer> keywordCounts) throws IOException {

        try (PrintWriter pw = new PrintWriter("reports.txt")) {
            pw.println("duplicates_found " + duplicatesFound.get());
            pw.println("unique_articles " + validArticles.size());

            Map.Entry<String, Integer> bestAuthor = getMaxEntry(authorCounts);
            if (bestAuthor != null)
                pw.println("best_author " + bestAuthor.getKey() + " " + bestAuthor.getValue());

            Map.Entry<String, Integer> topLang = getMaxEntry(langCounts);
            if (topLang != null)
                pw.println("top_language " + topLang.getKey() + " " + topLang.getValue());

            Map.Entry<String, Integer> topCat = getMaxEntry(catCounts);
            if (topCat != null) {
                String norm = topCat.getKey().replace(" ", "_").replace(",", "");
                pw.println("top_category " + norm + " " + topCat.getValue());
            }

            if (!validArticles.isEmpty()) {
                Article recent = validArticles.get(0);
                pw.println("most_recent_article " + recent.published + " " + recent.url);
            }

            Map.Entry<String, Integer> topKw = getMaxEntry(keywordCounts);
            if (topKw != null)
                pw.println("top_keyword_en " + topKw.getKey() + " " + topKw.getValue());
        }
    }

    private static Map.Entry<String, Integer> getMaxEntry(Map<String, Integer> map) {
        if (map.isEmpty()) return null;
        return map.entrySet().stream()
                .sorted((e1, e2) -> {
                    int comp = e2.getValue().compareTo(e1.getValue());
                    if (comp != 0) return comp;
                    return e1.getKey().compareTo(e2.getKey());
                })
                .findFirst()
                .orElse(null);
    }
}