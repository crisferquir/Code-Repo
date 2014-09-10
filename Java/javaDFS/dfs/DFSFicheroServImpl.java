// Clase de servidor que implementa el API de acceso remoto a un fichero

package dfs;
import java.rmi.*;
import java.rmi.server.*;

import java.io.*;

public class DFSFicheroServImpl extends UnicastRemoteObject implements DFSFicheroServ {
    private static final String DFSDir = "DFSDir/";
    private RandomAccessFile fichero;
    private String nombre; // <---------- nuevo

    public DFSFicheroServImpl()
      throws RemoteException, FileNotFoundException {
    }

	public DFSFicheroServImpl(String nombre, String modo)  throws RemoteException, FileNotFoundException{
		System.out.println("ruta: "+DFSDir+nombre);
		fichero = new RandomAccessFile(DFSDir+nombre, modo);
		this.nombre = nombre; // <---------- nuevo
	}

	@Override
	public void close() throws  IOException {
		fichero.close();
	}

	@Override
	public byte[] read(byte[] b) throws IOException {
		int res = fichero.read(b);
		System.out.println("Leo "+b+" Num "+res);
		if(res == -1){
			return new byte[0];			
		}
		return b;
	}

	@Override
	public long seek(long p) throws  IOException {
		fichero.seek(p);
		System.out.println("Me muevo a "+p);
		return p;
		
	}

	@Override
	public int write(byte[] b) throws IOException {
		fichero.write(b);
		System.out.println("Escribo "+b);
		return b.length;
	}
	public String getNombre() throws RemoteException, IOException{ // <---------- nuevo
		return DFSDir+nombre;
	}
}
