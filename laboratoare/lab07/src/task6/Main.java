package task6;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;

public class Main {
    public static int N = 4;

    static class QueensTask extends RecursiveAction {
        private final int[] graph;
        private final int step;

        public QueensTask(int[] graph, int step) {
            this.graph = graph;
            this.step = step;
        }

        @Override
        protected void compute() {
            if (Main.N == step) {
                printQueens(graph);
                return;
            }

            List<QueensTask> tasks = new ArrayList<>();

            for (int i = 0; i < Main.N; ++i) {
                int[] newGraph = graph.clone();
                newGraph[step] = i;

                if (check(newGraph, step)) {
                    tasks.add(new QueensTask(newGraph, step + 1));
                }
            }

            invokeAll(tasks);
        }
    }

    private static boolean check(int[] arr, int step) {
        for (int i = 0; i <= step; i++) {
            for (int j = i + 1; j <= step; j++) {
                if (arr[i] == arr[j] || arr[i] + i == arr[j] + j || arr[i] + j == arr[j] + i)
                    return false;
            }
        }
        return true;
    }

    private static synchronized void printQueens(int[] sol) {
        StringBuilder aux = new StringBuilder();
        for (int i = 0; i < sol.length; i++) {
            aux.append("(").append(sol[i] + 1).append(", ").append(i + 1).append("), ");
        }
        aux = new StringBuilder(aux.substring(0, aux.length() - 2));
        System.out.println("[" + aux + "]");
    }

    public static void main(String[] args) {
        ForkJoinPool fjp = new ForkJoinPool();
        int[] graph = new int[N];

        QueensTask rootTask = new QueensTask(graph, 0);

        fjp.invoke(rootTask);

        fjp.shutdown();
    }
}