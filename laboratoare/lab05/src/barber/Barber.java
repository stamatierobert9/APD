package barber;

public class Barber extends Thread {
    @Override
    public void run() {
        int servedClients = 0;

        do {
            try {
                // TODO
                Main.customers.acquire();
                Main.mutex.acquire();
                Main.chairs++;
                Main.mutex.release();
                Main.barber.release();
                // TODO

                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                System.out.println("Barber served client");
                ++servedClients;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        } while (checkIfThereAnyClients());
    }

    private boolean checkIfThereAnyClients() {
        int count = 0;
        for (var client: Main.leftClients) {
            if (client == 0) {
                count++;
            }
        }

        return count != 0;
    }
}
