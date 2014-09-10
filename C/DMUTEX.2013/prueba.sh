# El servidor es un recurso compartido, pero
# sólo hay un cliente que lo solicite.
# El semáforo que protege al servidor se denomina L
CLI: EVENT
# Antes de solicitar el servicio pide el semáforo
CLI: LOCK L
SRV: RECEIVE
CLI: RECEIVE
CLI: MESSAGETO SRV
SRV: RECEIVE
SRV: EVENT
SRV: MESSAGETO CLI
CLI: RECEIVE
# Ahora se libera el cerrojo.
CLI: UNLOCK L
CLI: GETCLOCK
SRV: GETCLOCK
>> Ejecutando: ./controlador CLISERV $TICK
>> Comparando salidas
>> Fichero: CLICLISERV
# El servidor es un recurso compartido, pero ahora
# hay dos clientes que lo solicitan.
# El semáforo que protege al servidor se denomina L
CL1: EVENT
CL2: EVENT
# Antes de solicitar el servicio pide el semáforo
CL1: LOCK L
CL2: RECEIVE
SRV: RECEIVE
CL1: RECEIVE
CL1: RECEIVE
CL1: MESSAGETO SRV
SRV: RECEIVE
SRV: EVENT
SRV: MESSAGETO CL1
CL1: RECEIVE
# Ahora se libera el cerrojo.
CL1: UNLOCK L
# Ahora lo hace le segundo cliente
CL2: LOCK L
SRV: RECEIVE
CL2: RECEIVE
CL1: RECEIVE
CL2: RECEIVE
CL2: MESSAGETO SRV
SRV: RECEIVE
SRV: EVENT
SRV: MESSAGETO CL2
CL2: RECEIVE
# Ahora se libera el cerrojo.
CL2: UNLOCK L
CL1: GETCLOCK
CL2: GETCLOCK
SRV: GETCLOCK
