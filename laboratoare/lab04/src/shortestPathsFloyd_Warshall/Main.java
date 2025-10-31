package shortestPathsFloyd_Warshall;

public class Main {

    public static void main(String[] args) {
        int N = 5;
        int M = 9;
        int P = 4;

        int[][] graph = {{0, 1, M, M, M},
                {1, 0, 1, M, M},
                {M, 1, 0, 1, 1},
                {M, M, 1, 0, M},
                {M, M, 1, M, 0}};

        Thread[] threads = new Thread[P];

        for (int k = 0; k < N; k++) {
            for (int i = 0; i < P; i++) {
                threads[i] = new Thread(new MyThread(i, P, N, k, graph));
                threads[i].start();
            }
            for (int i = 0; i < P; i++) {
                try {
                    threads[i].join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                System.out.print(graph[i][j] + " ");
            }
            System.out.println();
        }
    }
}