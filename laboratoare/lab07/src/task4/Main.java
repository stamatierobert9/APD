package task4;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;

public class Main {
    static int[][] graph = { { 0, 1 }, { 0, 4 }, { 0, 5 }, { 1, 0 }, { 1, 2 }, { 1, 6 }, { 2, 1 }, { 2, 3 }, { 2, 7 },
            { 3, 2 }, { 3, 4 }, { 3, 8 }, { 4, 0 }, { 4, 3 }, { 4, 9 }, { 5, 0 }, { 5, 7 }, { 5, 8 }, { 6, 1 },
            { 6, 8 }, { 6, 9 }, { 7, 2 }, { 7, 5 }, { 7, 9 }, { 8, 3 }, { 8, 5 }, { 8, 6 }, { 9, 4 }, { 9, 6 },
            { 9, 7 } };

    static class PathFinderTask extends RecursiveAction {
        private final ArrayList<Integer> partialPath;
        private final int destination;

        public PathFinderTask(ArrayList<Integer> partialPath, int destination) {
            this.partialPath = partialPath;
            this.destination = destination;
        }

        @Override
        protected void compute() {
            if (partialPath.get(partialPath.size() - 1) == destination) {
                synchronized (Main.class) {
                    System.out.println(partialPath);
                }
                return;
            }

            int lastNodeInPath = partialPath.get(partialPath.size() - 1);
            List<PathFinderTask> tasks = new ArrayList<>();

            for (int[] ints : graph) {
                if (ints[0] == lastNodeInPath) {
                    if (partialPath.contains(ints[1]))
                        continue;

                    ArrayList<Integer> newPartialPath = new ArrayList<>(partialPath);
                    newPartialPath.add(ints[1]);

                    PathFinderTask t = new PathFinderTask(newPartialPath, destination);
                    tasks.add(t);
                }
            }

            invokeAll(tasks);
        }
    }

    public static void main(String[] args) {
        ForkJoinPool fjp = new ForkJoinPool();

        ArrayList<Integer> partialPath = new ArrayList<>();
        partialPath.add(0);

        PathFinderTask rootTask = new PathFinderTask(partialPath, 3);

        fjp.invoke(rootTask);

        fjp.shutdown();
    }
}