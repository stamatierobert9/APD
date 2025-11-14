package multipleProducersMultipleConsumersNBuffer;

import java.util.Queue;

public class Buffer {
    
    private final Queue<Integer> queue;
    private final int capacity;
    
    public Buffer(int size) {
        queue = new LimitedQueue<>(size);
        capacity = size;
    }

	public void put(int value) {
        synchronized (this) {
            while (queue.size() == capacity) {
                try {
                    this.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            queue.add(value);
            this.notifyAll();
        }
	}

	public int get() {
        synchronized (this) {
            while (queue.isEmpty()) {
                try {
                    this.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            int value = queue.remove();
            this.notifyAll();
            return value;
        }
	}
}
