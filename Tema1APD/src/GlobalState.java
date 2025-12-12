import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class GlobalState {
    // Input lists
    public List<String> inputFiles = new ArrayList<>();
    public Set<String> targetLanguages = new HashSet<>();
    public Set<String> targetCategories = new HashSet<>();
    public Set<String> excludeWords = new HashSet<>();

    // Intermediate Data
    public List<Article> allRawArticles = Collections.synchronizedList(new ArrayList<>());

    // Duplicate detection counters (Concurrent for parallel updates)
    public ConcurrentHashMap<String, Integer> uuidCounts = new ConcurrentHashMap<>();
    public ConcurrentHashMap<String, Integer> titleCounts = new ConcurrentHashMap<>();

    // Final Processed Data
    public List<Article> cleanArticles = Collections.synchronizedList(new ArrayList<>());

    // Statistics
    public ConcurrentHashMap<String, AtomicInteger> keywordCounts = new ConcurrentHashMap<>();

    // Helper to add keyword count safely
    public void addKeyword(String word) {
        keywordCounts.computeIfAbsent(word, k -> new AtomicInteger(0)).incrementAndGet();
    }
}