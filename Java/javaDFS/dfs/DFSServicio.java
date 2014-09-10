// Interfaz del servicio DFS

package dfs;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.rmi.*;

public interface DFSServicio extends Remote {
	public FicheroInfo open(String nombre, String modo) throws RemoteException, FileNotFoundException, IOException;
}       
