package task1;

import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;

public class Main {
    // Graful rămâne neschimbat
    static int[][] graph = { { 0, 1 }, { 0, 4 }, { 0, 5 }, { 1, 0 }, { 1, 2 }, { 1, 6 }, { 2, 1 }, { 2, 3 }, { 2, 7 },
            { 3, 2 }, { 3, 4 }, { 3, 8 }, { 4, 0 }, { 4, 3 }, { 4, 9 }, { 5, 0 }, { 5, 7 }, { 5, 8 }, { 6, 1 },
            { 6, 8 }, { 6, 9 }, { 7, 2 }, { 7, 5 }, { 7, 9 }, { 8, 3 }, { 8, 5 }, { 8, 6 }, { 9, 4 }, { 9, 6 },
            { 9, 7 } };

    static ExecutorService executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());

    static AtomicInteger activeTasks = new AtomicInteger(0);

    static void getPath(ArrayList<Integer> partialPath, int destination) {
        if (partialPath.get(partialPath.size() - 1) == destination) {
            synchronized (Main.class) {
                System.out.println(partialPath);
            }
            if (activeTasks.decrementAndGet() == 0) {
                executor.shutdown();
            }
            return;
        }

        int lastNodeInPath = partialPath.get(partialPath.size() - 1);

        for (int[] ints : graph) {
            if (ints[0] == lastNodeInPath) {
                if (partialPath.contains(ints[1]))
                    continue;

                ArrayList<Integer> newPartialPath = new ArrayList<>(partialPath);
                newPartialPath.add(ints[1]);

                activeTasks.incrementAndGet();

                executor.submit(() -> getPath(newPartialPath, destination));
            }
        }

        if (activeTasks.decrementAndGet() == 0) {
            executor.shutdown();
        }
    }

    public static void main(String[] args) {
        ArrayList<Integer> partialPath = new ArrayList<>();
        partialPath.add(0);

        activeTasks.incrementAndGet();

        executor.submit(() -> getPath(partialPath, 3));

        while (!executor.isTerminated()) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}