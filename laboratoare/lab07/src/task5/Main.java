package task5;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;

public class Main {
    static int N = 10;
    static int COLORS = 3;
    static int[][] graph = { { 0, 1 }, { 0, 4 }, { 0, 5 }, { 1, 0 }, { 1, 2 }, { 1, 6 }, { 2, 1 }, { 2, 3 }, { 2, 7 },
            { 3, 2 }, { 3, 4 }, { 3, 8 }, { 4, 0 }, { 4, 3 }, { 4, 9 }, { 5, 0 }, { 5, 7 }, { 5, 8 }, { 6, 1 },
            { 6, 8 }, { 6, 9 }, { 7, 2 }, { 7, 5 }, { 7, 9 }, { 8, 3 }, { 8, 5 }, { 8, 6 }, { 9, 4 }, { 9, 6 },
            { 9, 7 } };

    // Definim task-ul recursiv pentru colorare
    static class ColorTask extends RecursiveAction {
        private final int[] colors;
        private final int step;

        public ColorTask(int[] colors, int step) {
            this.colors = colors;
            this.step = step;
        }

        @Override
        protected void compute() {
            if (step == N) {
                printColors(colors);
                return;
            }

            List<ColorTask> tasks = new ArrayList<>();

            for (int i = 0; i < COLORS; i++) {
                int[] newColors = colors.clone();
                newColors[step] = i;

                if (verifyColors(newColors, step)) {
                    tasks.add(new ColorTask(newColors, step + 1));
                }
            }

            invokeAll(tasks);
        }
    }

    private static boolean verifyColors(int[] colors, int step) {
        for (int i = 0; i < step; i++) {
            if (colors[i] == colors[step] && isEdge(i, step))
                return false;
        }
        return true;
    }

    private static boolean isEdge(int a, int b) {
        for (int[] ints : graph) {
            if (ints[0] == a && ints[1] == b)
                return true;
        }
        return false;
    }

    static synchronized void printColors(int[] colors) {
        StringBuilder aux = new StringBuilder();
        for (int color : colors) {
            aux.append(color).append(" ");
        }
        System.out.println(aux);
    }

    public static void main(String[] args) {
        ForkJoinPool fjp = new ForkJoinPool();
        int[] colors = new int[N];

        ColorTask rootTask = new ColorTask(colors, 0);
        fjp.invoke(rootTask);

        fjp.shutdown();
    }
}