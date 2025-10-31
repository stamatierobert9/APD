package doubleVectorElements;

public class Main {

    public static void main(String[] args) {
        int N = 100000013;
        int[] v = new int[N];
        int P = 4;

        for (int i = 0; i < N; i++) {
            v[i] = i;
        }

        Thread[] threads = new Thread[P];

        for (int i = 0; i < P; i++) {
            threads[i] = new Thread(new MyThread(i, P, N, v));
            threads[i].start();
        }

        for (int i = 0; i < P; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        for (int i = 0; i < N; i++) {
            if (v[i] != i * 2) {
                System.out.println("Wrong answer");
                System.exit(1);
            }
        }
        System.out.println("Correct");
    }
}