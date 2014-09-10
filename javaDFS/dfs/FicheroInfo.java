// Esta clase representa información de un fichero.
// El enunciado explica más en detalle el posible uso de esta clase.
// Al ser serializable, puede usarse en las transferencias entre cliente
// y servidor.

package dfs;
import java.io.*;
import java.rmi.RemoteException;
import java.util.Date;

public class FicheroInfo implements Serializable {
	private DFSFicheroServ fichero;
	private long fecha;
	private String nombre;// <---------- nuevo

	public FicheroInfo(DFSFicheroServ fichero) throws RemoteException, IOException {
		this.fichero = fichero;
		this.nombre = fichero.getNombre();// <---------- nuevo
		File f = new File(nombre);// <---------- nuevo		
		this.fecha = f.lastModified();// <---------- nuevo
		nombre = fichero.getNombre();// <---------- nuevo
	}
	public DFSFicheroServ getFichero() {
		return fichero;
	}
	public void setFichero(DFSFicheroServ fs){// <---------- nuevo ...creo
		this.fichero = fs;
	}
	public long getFecha() {
		return fecha;
	}
	public void setFecha(long f){// <---------- nuevo
		this.fecha = f;
	}

	public void setLastModified(long time) {// <---------- nuevo
		this.fecha = time;
		File f = new File(nombre);
		f.setLastModified(time);		
	}
}
