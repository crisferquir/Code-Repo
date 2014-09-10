// Interfaz del API de acceso remoto a un fichero

package dfs;
import java.io.IOException;
import java.rmi.*;

public interface DFSFicheroServ extends Remote  {
	
    public byte[] read(byte[] b) throws RemoteException, IOException;
       public int write(byte[] b) throws RemoteException, IOException;
       public long seek(long p) throws RemoteException, IOException;
       public void close() throws RemoteException, IOException ;
       public String getNombre() throws RemoteException, IOException; // <---------- nuevo
}
