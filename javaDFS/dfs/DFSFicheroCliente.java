// Clase de cliente que proporciona el API del servicio DFS

package dfs;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.rmi.*;
import java.util.Date;
import java.util.List;



public class DFSFicheroCliente  {
	private DFSCliente dfs;
	private String nombre,modo;
	private FicheroInfo fichero;
	private long cursor;
	private boolean abierto;
	
    public DFSFicheroCliente(DFSCliente dfs, String nom, String modo) throws RemoteException, IOException, FileNotFoundException {
    	this.nombre = nom;
    	this.modo = modo;    	
    	this.dfs = dfs;    	
    	fichero = dfs.getDfs().open(nom, modo);
    	abierto = true;
    	cursor = 0;
    	if(!dfs.existCache(nom)){
    		dfs.addCache(nom);
    	}
    	System.out.println("FECHA DEL FICHERO ABIERTO: "+fichero.getFecha()+"FECHA CACHE: "+dfs.getmCaches(nom).obtenerFecha());
    	if(fichero.getFecha()>dfs.getmCaches(nom).obtenerFecha()){
    		dfs.getmCaches(nom).vaciar();
    	}    	
    	
    }
    public int read(byte[] b) throws RemoteException, IOException {
		if (!abierto){
			throw new IOException("El fichero no esta abierto");
		}
		// Comprobar cache
		Cache cache = dfs.getmCaches(nombre);
		int tBloque = dfs.getTamBloque(); 
		int nBloques = b.length/tBloque; // número de bloques a leer
		long posOld=cursor;
		
		byte[] result = new byte[b.length]; // contenedor 
		for (int i=0;i<nBloques;i++){ // mientras no hemos leído toooo
			int pos = i*tBloque;//Posicion de inicio del bloque
			Bloque bloq = cache.getBloque(cursor); //traer bloque de cache 
			if (bloq!=null){//Existe en la cache
				byte[] leer = new byte[tBloque];
				leer = bloq.obtenerContenido();
				System.arraycopy(leer, 0, b, pos, tBloque);
				seek(cursor+tBloque);
			}else{//No existe en la cache
				byte[] leer = new byte[tBloque];
				leer = fichero.getFichero().read(leer);
				cursor+=tBloque;	
		        if (leer.length==0){
		        	cursor=posOld;
		            return -1;
		        }
		        Bloque bloque = new Bloque(cursor-tBloque, leer);
		        Bloque expulsado = cache.putBloque(bloque);
		        cache.activarMod(bloque);
				if (expulsado!=null){//Cache llena
					if (cache.preguntarMod(expulsado)){
						fichero.getFichero().seek(expulsado.obtenerId());
						fichero.getFichero().write(expulsado.obtenerContenido());
						fichero.getFichero().seek(cursor);
						cache.desactivarMod(expulsado);
					}
				}
		        cache.preguntarYDesactivarMod(bloque);
				System.arraycopy(leer, 0, b, pos, tBloque);
			}
		}
		return result.length;
    }
    public void write(byte[] b) throws RemoteException, IOException {
    	if(!abierto){
    		throw new IOException("FICHERO NO ABIERTO!!!");
    	}
    	if(modo.equals("r")){
    		throw new IOException("SOLO LECTURA");
    	}
		Cache cache = dfs.getmCaches(nombre);
		int tBloque = dfs.getTamBloque();// tamaño de bloques 
		int nBloques = b.length/tBloque; // números de bloques a escribir
		
		
		for (int i=0;i<nBloques;i++){ // por cada bloque a escribir
			byte[] bActual = new byte[tBloque]; // creo un bloque nuevo
			System.arraycopy(b, i*tBloque, bActual, 0, tBloque); // copio el contenido del bloque que quiero escribir al nuevo
			Bloque bloque = new Bloque (cursor,bActual); // creo un bloque que tiene de id el cursor
			Bloque expulsado = cache.putBloque(bloque); // >> !! modificado CACHE !! <<
			if (expulsado!=null){ // Si no hay sitio en la cache
				if (cache.preguntarMod(expulsado)){ // comprobar si el expulsado había sido modificado (para escribirlo en el fichero)
					fichero.getFichero().seek(expulsado.obtenerId());
					fichero.getFichero().write(expulsado.obtenerContenido());
					fichero.getFichero().seek(cursor);
					cache.desactivarMod(expulsado);
					fichero.setLastModified(new Date().getTime()); // <---------- nuevo
				}
			}
			cache.activarMod(bloque); // avisar de que el bloque hay que volver a escribirlo en el fichero
			cursor+=tBloque; // avanzar el cursor
			fichero.getFichero().seek(cursor); 
		}
    }
    public void seek(long p) throws RemoteException, IOException {
    	if(!abierto){
    		throw new IOException("FICHERO NO ABIERTO!!!");
    	}
    	cursor = p;
    	fichero.getFichero().seek(p);
    	
    }
    public void close() throws RemoteException, IOException {
    	if(!abierto){
    		throw new IOException("FICHERO NO ABIERTO!!!");
    	}
    	Cache cache = dfs.getmCaches(nombre);
    	List<Bloque> ls = cache.listaMod();
		Date date = new Date();// <---------- nuevo
    	
    	for (Bloque b: ls){
    		System.out.println("BLOQUE MODIFICADO");
    		fichero.getFichero().seek(b.obtenerId());
    		fichero.getFichero().write(b.obtenerContenido());
    		fichero.setLastModified(date.getTime());// <---------- nuevo
    	}
    	cache.vaciarListaMod();
    	abierto = false;
    	cache.fijarFecha(fichero.getFecha());
    	System.out.println("fecha: "+fichero.getFecha());
    	fichero.setLastModified(date.getTime());// <---------- nuevo
    	fichero.getFichero().close();
    	
    }
}
