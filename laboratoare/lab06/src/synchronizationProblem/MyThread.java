package synchronizationProblem;

public class MyThread implements Runnable {
	private final int id;
	public static int value;

	public MyThread(int id) {
		this.id = id;
	}

	@Override
	public void run() {
		if (id == 0) {
			for (int i = 0; i < Main.N; i++) {
				value += 3;
			}
		} else {
			for (int i = 0; i < Main.N; i++) {
				value += 3;
			}
		}
	}
}
