package barber;

public class Client extends Thread {
    private final int id;

    public Client(int id) {
        super();
        this.id = id;
    }

    @Override
    public void run() {
        try {
            // TODO
            Main.mutex.acquire();

            if (Main.chairs > 0) {
                // client occupies a seat
                Main.chairs--;

                System.out.println("Client " + id + " is waiting for haircut");
                System.out.println("Available seats: " + Main.chairs);

                // TODO
                Main.customers.release();
                Main.mutex.release();
                Main.barber.acquire();

                System.out.println("Client " + id + " is served by the barber");

                Main.leftClients[id] = Main.SERVED_CLIENT;
            } else {
                // TODO
                Main.mutex.release();
                System.out.println("Client " + id + " left unserved");
                Main.leftClients[id] = Main.UNSERVED_CLIENT;
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
