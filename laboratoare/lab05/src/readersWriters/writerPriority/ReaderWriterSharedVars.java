package readersWriters.writerPriority;

public class ReaderWriterSharedVars {
    // The value to read/write
    volatile int shared_value;
    // TODO: Add semaphores and anything else needed for synchronization
    // Semaphore s;

    private int ActiveReaders = 0;
    private int WaitingWriters = 0;
    private boolean isWritting = false;

    public synchronized void startRead() throws InterruptedException {
        while (WaitingWriters > 0 || isWritting) {
            wait();
        }
        ActiveReaders++;
    }

    public synchronized void stopRead() throws InterruptedException {
        ActiveReaders--;
        if (ActiveReaders == 0) {
            notifyAll();
        }
    }

    public synchronized void startWrite() throws InterruptedException {
        WaitingWriters++;
        while (ActiveReaders > 0 || isWritting) {
            wait();
        }
        WaitingWriters--;
        isWritting = true;
    }

    public synchronized void stopWrite() throws InterruptedException {
        isWritting = false;
        notifyAll();
    }

    ReaderWriterSharedVars(int init_shared_value) {
        this.shared_value = init_shared_value;
        //this.s = new Semaphore(1);
    }

}
