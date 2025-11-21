package synchronizedSortedList;

import java.util.Collections;
import java.util.List;
import java.util.concurrent.Semaphore;

public class Sort extends Thread {
    private final List<Integer> list;
    private final Semaphore semBarrier;

    public Sort(List<Integer> list, Semaphore semBarrier) {
        this.list = list;
        this.semBarrier = semBarrier;
    }

    @Override
    public void run() {
        try {
            semBarrier.acquire(3);

            Collections.sort(list);

        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}