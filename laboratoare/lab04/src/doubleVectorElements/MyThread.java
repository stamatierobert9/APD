package doubleVectorElements;

public class MyThread implements Runnable {
    int threadId;
    int P;
    int N;
    int[] v;

    public MyThread(int threadId, int P, int N, int[] v) {
        this.threadId = threadId;
        this.P = P;
        this.N = N;
        this.v = v;
    }

    @Override
    public void run() {
        int chunkSize = N / P;
        int remainder = N % P;

        int start = threadId * chunkSize + Math.min(threadId, remainder);
        int end = (threadId + 1) * chunkSize + Math.min(threadId + 1, remainder);

        for (int i = start; i < end; i++) {
            v[i] = v[i] * 2;
        }
    }
}