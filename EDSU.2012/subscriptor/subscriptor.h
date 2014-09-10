  /* --------------DECLARACIÓN DE FUNCIONES -------------*/
  int alta_subscripcion_tema(const char *tema);
  int baja_subscripcion_tema(const char *tema);
  int inicio_subscriptor(void (*notif_evento)(const char *, const char *),
                  void (*alta_tema)(const char *),
                  void (*baja_tema)(const char *));
  int fin_subscriptor();  
