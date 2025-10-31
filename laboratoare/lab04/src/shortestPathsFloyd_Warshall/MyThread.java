package shortestPathsFloyd_Warshall;

public class MyThread implements Runnable {
    private final int threadId;
    private final int P;
    private final int N;
    private final int k;
    private final int[][] graph;

    public MyThread(int threadId, int P, int N, int k, int[][] graph) {
        this.threadId = threadId;
        this.P = P;
        this.N = N;
        this.k = k;
        this.graph = graph;
    }

    @Override
    public void run() {
        int chunkSize = N / P;
        int remainder = N % P;
        int start = threadId * chunkSize + Math.min(threadId, remainder);
        int end = (threadId + 1) * chunkSize + Math.min(threadId + 1, remainder);

        for (int i = start; i < end; i++) {
            for (int j = 0; j < N; j++) {
                graph[i][j] = Math.min(graph[i][k] + graph[k][j], graph[i][j]);
            }
        }
    }
}