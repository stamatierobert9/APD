import java.util.ArrayList;
import java.util.List;

public class Article implements Comparable<Article> {
    public String uuid;
    public String title;
    public String author;
    public String published;
    public String url;
    public String text;
    public String language;
    public List<String> categories;

    public Article() {
        this.categories = new ArrayList<>();
    }

    // Pentru sortarea cronologică descrescătoare (cel mai recent primul)
    // La egalitate de timp, se sortează lexicografic după UUID [cite: 73]
    @Override
    public int compareTo(Article other) {
        // String comparison merge direct pe formatul ISO8601 UTC [cite: 87]
        int dateComparison = other.published.compareTo(this.published);
        if (dateComparison != 0) {
            return dateComparison;
        }
        return this.uuid.compareTo(other.uuid);
    }

    @Override
    public String toString() {
        return "Article{" + "uuid='" + uuid + '\'' + ", title='" + title + '\'' + '}';
    }
}