// Clase de cliente que proporciona acceso al servicio DFS

package dfs;

import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.util.HashMap;

public class DFSCliente {
	private int  tamBloque;
	private int  tamCache;
	private DFSServicio dfs;
	private HashMap <String, Cache> mCaches;
	
    public DFSCliente(int tamBloque, int tamCache) {
    	this.tamBloque = tamBloque;
    	this.tamCache  = tamCache;
    	String host = System.getenv("SERVIDOR");
    	String puerto = System.getenv("PUERTO");
    	try {
			dfs = (DFSServicio) Naming.lookup("//"+host+":"+puerto+"/DFS");
		} catch (MalformedURLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (RemoteException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NotBoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	mCaches = new HashMap<String,Cache>();
    }

	public DFSServicio getDfs() {
		return dfs;
	}
	public void addCache(String nombre){
		mCaches.put(nombre, new Cache(tamCache));
	}

	public Cache getmCaches(String nombre) {
		return mCaches.get(nombre);
	}
	public boolean existCache(String nombre){
		return mCaches.containsKey(nombre);
	}


	public int getTamBloque() {
		return tamBloque;
	}

	public int getTamCache() {
		return tamCache;
	}

}

