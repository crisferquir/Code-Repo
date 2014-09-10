// Clase de servidor que implementa el servicio DFS

package dfs;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.rmi.*;
import java.rmi.server.*;
import java.util.Date;
import java.util.HashMap;

public class DFSServicioImpl extends UnicastRemoteObject implements DFSServicio {
	
	
    public DFSServicioImpl() throws RemoteException {

    }

	@Override
	public FicheroInfo open(String nombre, String modo) throws IOException {
		DFSFicheroServ fichero = new DFSFicheroServImpl(nombre,modo);
		FicheroInfo fichInfo = new FicheroInfo(fichero);
		return fichInfo;	
		
	}
	
}
