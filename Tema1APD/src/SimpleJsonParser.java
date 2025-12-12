import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class SimpleJsonParser {

    // Regex simplificat pentru extragerea valorilor dintre ghilimele
    // Tratează și ghilimelele escaped (\") din interiorul textului
    private static final Pattern STRING_FIELD_PATTERN = Pattern.compile("\"(uuid|title|author|published|url|text|language)\"\\s*:\\s*\"((?:[^\"\\\\]|\\\\.)*)\"");
    private static final Pattern LIST_FIELD_PATTERN = Pattern.compile("\"categories\"\\s*:\\s*\\[(.*?)\\]");

    public static List<Article> parseFile(String filePath) throws IOException {
        List<Article> articles = new ArrayList<>();
        StringBuilder contentBuilder = new StringBuilder();

        try (BufferedReader br = new BufferedReader(new FileReader(filePath))) {
            String line;
            while ((line = br.readLine()) != null) {
                contentBuilder.append(line).append("\n");
            }
        }

        String content = contentBuilder.toString();

        // Split grosier după obiecte JSON. Presupunem că articolele sunt într-un array [...]
        // și sunt delimitate de }, {
        String[] objects = content.split("(?<=\\}),\\s*(?=\\{)");

        for (String objStr : objects) {
            // Curățăm parantezele pătrate de început/sfârșit de fișier dacă există
            objStr = objStr.trim();
            if (objStr.startsWith("[")) objStr = objStr.substring(1);
            if (objStr.endsWith("]")) objStr = objStr.substring(0, objStr.length() - 1);

            Article article = new Article();

            // Extragere câmpuri simple
            Matcher m = STRING_FIELD_PATTERN.matcher(objStr);
            while (m.find()) {
                String key = m.group(1);
                String value = unescape(m.group(2));

                switch (key) {
                    case "uuid": article.uuid = value; break;
                    case "title": article.title = value; break;
                    case "author": article.author = value; break;
                    case "published": article.published = value; break;
                    case "url": article.url = value; break;
                    case "text": article.text = value; break;
                    case "language": article.language = value; break;
                }
            }

            // Extragere categorii
            Matcher mCat = LIST_FIELD_PATTERN.matcher(objStr);
            if (mCat.find()) {
                String arrayContent = mCat.group(1);
                String[] cats = arrayContent.split(",");
                for (String c : cats) {
                    String catClean = c.trim().replace("\"", "");
                    if (!catClean.isEmpty()) {
                        article.categories.add(catClean);
                    }
                }
            }

            // Validare minimă: dacă are uuid, îl adăugăm
            if (article.uuid != null) {
                articles.add(article);
            }
        }
        return articles;
    }

    private static String unescape(String input) {
        return input.replace("\\\"", "\"").replace("\\\\", "\\");
    }
}