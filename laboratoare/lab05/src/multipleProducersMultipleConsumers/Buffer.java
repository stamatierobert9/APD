package multipleProducersMultipleConsumers;

public class Buffer {
    private int a;
    private boolean isEmpty = true;

    public synchronized void put(int value) {
        while (!isEmpty) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        a = value;
        isEmpty = false;
        notifyAll();
    }

    public synchronized int get() {
        while (isEmpty) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        isEmpty = true;
        notifyAll();
        return a;
    }
}
