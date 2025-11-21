package synchronizedSortedList;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.List;
import java.util.Scanner;
import java.util.concurrent.Semaphore;

public class Reader extends Thread {
    private final String filename;
    private final List<Integer> list;
    private final Semaphore semBarrier;
    private final Semaphore semMutex;

    public Reader(String filename, List<Integer> list, Semaphore semBarrier, Semaphore semMutex) {
        this.filename = filename;
        this.list = list;
        this.semBarrier = semBarrier;
        this.semMutex = semMutex;
    }

    @Override
    public void run() {
        try {
            Scanner scanner = new Scanner(new File(filename));
            while (scanner.hasNextInt()) {
                int number = scanner.nextInt();

                try {
                    semMutex.acquire();
                    list.add(number);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                } finally {
                    semMutex.release();
                }
            }
            scanner.close();

            semBarrier.release();

        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }
}