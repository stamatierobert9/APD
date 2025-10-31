package ex1;

public class Main {

    public static void main(String[] args) {

        int cores = Runtime.getRuntime().availableProcessors();

        System.out.println("Sistemul are " + cores + " core-uri. Se lansează thread-urile...");

        Thread[] threads = new Thread[cores];

        for (int i = 0; i < cores; i++) {
            Runnable task = new MyThread(i);

            threads[i] = new Thread(task);

            threads[i].start();
        }

        for (int i = 0; i < cores; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        System.out.println("Toate thread-urile și-au terminat execuția.");
    }
}
