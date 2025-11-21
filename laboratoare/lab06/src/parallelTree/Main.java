package parallelTree;

import java.util.concurrent.CyclicBarrier;

public class Main {
	static int N_ITERATIONS = 100;

	public static void main(String[] args) {
		Thread[] threads = new Thread[3];
		System.out.println("Parallel tree problem");
		
		for (int j = 0; j < N_ITERATIONS; j++) {
			TreeNode tree = new TreeNode(1);
            CyclicBarrier barrier = new CyclicBarrier(3);

			threads[0] = new Thread(new ReadTreePart(tree, "laboratoare/lab06/treePart1.txt", barrier));
			threads[1] = new Thread(new ReadTreePart(tree, "laboratoare/lab06/treePart2.txt", barrier));
			threads[2] = new Thread(new VerifyTree(tree, barrier));
			for (int i = 0; i < 3; i++) {
				threads[i].start();
			}

			for (int i = 0; i < 3; i++) {
				try {
					threads[i].join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
}