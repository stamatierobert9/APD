package parallelTree;

public class TreeNode {
	int name;
	public TreeNode left = null;
	public TreeNode right = null;

	public TreeNode(int name) {
		this.name = name;
	}

	public synchronized void addChild(TreeNode child) {
		if (left == null) {
			left = child;
			return;
		}
		right = child;
	}

	public synchronized TreeNode getNode(int name) {
		TreeNode aux = null;

		if (this.name == name) {
			return this;
		}

		if (left != null) {
			aux = left.getNode(name);
		}

		if (aux != null) {
			return aux;
		}

		if (right != null) {
			aux = right.getNode(name);
		}

		return aux;
	}
}
