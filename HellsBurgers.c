#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>


#define LIMITE 50

//declaramos los mutex
pthread_mutex_t plancha;
pthread_mutex_t salero;
pthread_mutex_t horno;


//verifica al ganador
int ganador=0;
int ganador;

//creo estructura de semaforos 
struct semaforos {
    sem_t sem_mezclar;
    	//poner demas semaforos aqui
	sem_t sem_salar;
    sem_t sem_armarMedallones;    
    sem_t sem_plancharMedallones;
    sem_t sem_hornearPan;
    sem_t sem_cortarLechugaYTomate;
    sem_t sem_armarHamburguesas;
    

};

//creo los pasos con los ingredientes
struct paso {
   	char accion [LIMITE];
 	char ingredientes[5][LIMITE];
   
};

//creo los parametros de los hilos 
struct parametro {
 	int equipo_param;
 
 	FILE *resultado;
 
 	struct semaforos semaforos_param;
 	struct paso pasos_param[8];
};

//funcion para imprimir las acciones y los ingredientes de la accion
void* imprimirAccion(void *data, char *accionIn) {	
	struct parametro *mydata = data;

	FILE *resultado= mydata->resultado;
	//calculo la longitud del array de pasos 
	int sizeArray = (int)( sizeof(mydata->pasos_param) / sizeof(mydata->pasos_param[0]));
	//indice para recorrer array de pasos 
	int i;
	for(i = 0; i < sizeArray; i ++){
		//pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
		if(strcmp(mydata->pasos_param[i].accion, accionIn) == 0){
		printf("Equipo %d - %s " , mydata->equipo_param, mydata->pasos_param[i].accion);
		fprintf(resultado,"Equipo %d - %s " , mydata->equipo_param, mydata->pasos_param[i].accion);
		//calculo la longitud del array de ingredientes
		int sizeArrayIngredientes = (int)( sizeof(mydata->pasos_param[i].ingredientes) / sizeof(mydata->pasos_param[i].ingredientes[0]) );
		//indice para recorrer array de ingredientes
		int h;		 
			for(h = 0; h < sizeArrayIngredientes; h++) {
				//consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
				if(strlen(mydata->pasos_param[i].ingredientes[h]) != 0) {
							printf("%s ",mydata->pasos_param[i].ingredientes[h]);
							fprintf(resultado,"%s ",mydata->pasos_param[i].ingredientes[h]);
				}
			}			
		}
	}
}
//funcion para tomar de ejemplo
void* cortar(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "cortar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;	
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//uso sleep para simular que que pasa tiempo
	usleep( 20000 );
	//doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_mezclar);
   
    pthread_exit(NULL);
}
void* mezclar(void *data) {
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_mezclar);
	char *accion = "mezclar";	
	imprimirAccion(mydata,accion);
	usleep( 200000 );
    sem_post(&mydata->semaforos_param.sem_salar);
    pthread_exit(NULL);
}
void* salar(void *data) {
	pthread_mutex_lock(&salero);
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_salar);
	char *accion = "salar";	
	imprimirAccion(mydata,accion);
	usleep( 100000 );
    sem_post(&mydata->semaforos_param.sem_armarMedallones);
    pthread_mutex_unlock(&salero);
    pthread_exit(NULL);
}
void* hornear(void *data) {
	pthread_mutex_lock(&horno);
	char *accion = "hornear";
	struct parametro *mydata = data;
	FILE *resultado= mydata->resultado;
	imprimirAccion(mydata,accion);	
	usleep( 500000 );
	printf("Equipo %d - panes listos\n " , mydata->equipo_param);
	fprintf(resultado,"Equipo %d - panes listos\n " , mydata->equipo_param);
    	sem_post(&mydata->semaforos_param.sem_hornearPan);
    	pthread_mutex_unlock(&horno);
    	pthread_exit(NULL);
}
void* armar(void *data) {
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_armarMedallones);
	char *accion = "armar";
	imprimirAccion(mydata,accion);
	usleep( 300000 );
    	sem_post(&mydata->semaforos_param.sem_plancharMedallones);
    	pthread_exit(NULL);
}

void* planchar(void *data) {
	pthread_mutex_lock(&plancha);
	struct parametro *mydata = data;
	FILE *resultado= mydata->resultado;
	sem_wait(&mydata->semaforos_param.sem_plancharMedallones);
	char *accion = "planchar";
	imprimirAccion(mydata,accion);
	usleep( 1000000 );
	printf("Equipo %d - 1er medallon listo\n " , mydata->equipo_param);
	fprintf(resultado,"Equipo %d - 1er medallon listo\n " , mydata->equipo_param);
	imprimirAccion(mydata,accion);
	usleep( 1000000 );
	printf("Equipo %d - 2do medallon listo\n " , mydata->equipo_param);
	fprintf(resultado,"Equipo %d - 2do medallon listo\n " , mydata->equipo_param);		
    sem_post(&mydata->semaforos_param.sem_cortarLechugaYTomate);
    pthread_mutex_unlock(&plancha);	
    pthread_exit(NULL);
}
void* preparar(void *data) {	
	char *accion = "preparar";
	struct parametro *mydata = data;
	imprimirAccion(mydata,accion);
	usleep( 100000 );	
    sem_post(&mydata->semaforos_param.sem_armarHamburguesas);
    pthread_exit(NULL);
}

void* terminar(void *data) {
	struct parametro *mydata = data;
	FILE *resultado= mydata->resultado;
	sem_wait(&mydata->semaforos_param.sem_cortarLechugaYTomate);
	sem_wait(&mydata->semaforos_param.sem_hornearPan);
	sem_wait(&mydata->semaforos_param.sem_armarHamburguesas);
	char *accion = "terminar";
	imprimirAccion(mydata,accion);
	usleep(400000 );
	printf("Equipo %d - Primer hamburguesa lista\n " , mydata->equipo_param);
	fprintf(resultado,"Equipo %d - Primer hamburguesa lista\n " , mydata->equipo_param);
	imprimirAccion(mydata,accion);
	usleep(400000 );
	printf("Equipo %d - Segunda hamburguesa lista\n " , mydata->equipo_param);
	fprintf(resultado,"Equipo %d - Segunda hamburguesa lista\n " , mydata->equipo_param);
	if (ganador!=1){
		ganador=mydata->equipo_param;
		}
	ganador=1;		
    	pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
	
	//variables semaforos
	sem_t sem_mezclar;
    	sem_t sem_salar;
    	sem_t sem_armarMedallones;
   	sem_t sem_plancharMedallones;
   	sem_t sem_hornearPan;
    	sem_t sem_cortarLechugaYTomate;
    	sem_t sem_armarHamburguesas;
	//crear variables semaforos aqui
	
	//variables hilos
	pthread_t p1;
	pthread_t p2;
	pthread_t p3;
	pthread_t p4;
	pthread_t p5;
	pthread_t p6;
	pthread_t p7;
	pthread_t p8; 
	//crear variables hilos aqui
	
	
	
	//reservo memoria para el struct
	struct parametro *pthread_data = i;

	//seteo los otros valores al struct
	
	
	//seteo semaforos
	
	pthread_data->semaforos_param.sem_mezclar = sem_mezclar;
    	pthread_data->semaforos_param.sem_salar = sem_salar;    
    	pthread_data->semaforos_param.sem_armarMedallones = sem_armarMedallones;
    	pthread_data->semaforos_param.sem_plancharMedallones = sem_plancharMedallones;
    	pthread_data->semaforos_param.sem_hornearPan = sem_hornearPan;
    	pthread_data->semaforos_param.sem_cortarLechugaYTomate = sem_cortarLechugaYTomate;
    	pthread_data->semaforos_param.sem_armarHamburguesas = sem_armarHamburguesas;
	//setear demas semaforos al struct aqui	

	//lee nuestro archivo receta.txt
	FILE *receta;	
	receta= fopen("receta.txt","r");
	char linea[LIMITE];
	//seteo las acciones de los ingredientes a usar		
	int j=0;		
    	while(fgets(linea, LIMITE, (FILE*) receta)){
    	char delim[] = " ";    		
    	char *ptr = strtok(linea, delim);					
		strcpy(pthread_data->pasos_param[j].accion,ptr);		
		int i=0;
		while (ptr != NULL){
			if(strcmp(pthread_data->pasos_param[j].accion, ptr)!=0){
				strcpy(pthread_data->pasos_param[j].ingredientes[i], ptr);				
			}
			ptr = strtok(NULL, delim);
			i++;		
		}
		j++;		
	}   

	//inicializo los semaforos
	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
    	sem_init(&(pthread_data->semaforos_param.sem_salar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armarMedallones),0,0);
    	sem_init(&(pthread_data->semaforos_param.sem_plancharMedallones),0,0); 
    	sem_init(&(pthread_data->semaforos_param.sem_hornearPan),0,0); 
    	sem_init(&(pthread_data->semaforos_param.sem_cortarLechugaYTomate),0,0);
    	sem_init(&(pthread_data->semaforos_param.sem_armarHamburguesas ),0,0);


	//creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    int rc;
    rc = pthread_create(&p1,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia	
	rc = pthread_create(&p2,                           
                            NULL,                         
                                mezclar,            
                                pthread_data);                                
	rc = pthread_create(&p3,                           
                            NULL,                         
                                hornear,            
                                pthread_data);
	rc = pthread_create(&p4,                           
                            NULL,                         
                                salar,            
                                pthread_data);                                
	rc = pthread_create(&p5,                           
                            NULL,                         
                                preparar,            
                                pthread_data);
	rc = pthread_create(&p6,                           
                            NULL,                         
                                planchar,            
                                pthread_data);
                                
	rc = pthread_create(&p7,                           
                            NULL,                         
                                armar,            
                                pthread_data);
	rc = pthread_create(&p8,                           
                            NULL,                         
                                terminar,            
                                pthread_data);  
	                                                                                                                                        
	pthread_join (p1,NULL);
	pthread_join (p2,NULL);
	pthread_join (p3,NULL);
	pthread_join (p4,NULL);
	pthread_join (p5,NULL);
	pthread_join (p6,NULL);
	pthread_join (p7,NULL);
	pthread_join (p8,NULL);
	
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     }
	 
	//destruccion de los semaforos 
	sem_destroy(&sem_mezclar);
	sem_destroy(&sem_salar);   
    sem_destroy(&sem_armarMedallones);
    sem_destroy(&sem_plancharMedallones);
    sem_destroy(&sem_hornearPan);
    sem_destroy(&sem_cortarLechugaYTomate);
    sem_destroy(&sem_armarHamburguesas);
	//destruir demas semaforos 
	
	
	
	//salida del hilo
	pthread_exit(NULL);
}

int main (){
		 
	int rc;
	FILE *resultado;	
	resultado = fopen("resultado.txt","w+");
	struct parametro *equipo1Param =malloc(sizeof(struct parametro));
	struct parametro *equipo2Param =malloc(sizeof(struct parametro));
	struct parametro *equipo3Param =malloc(sizeof(struct parametro));	
	equipo1Param->resultado = resultado;
	equipo2Param->resultado = resultado;
	equipo3Param->resultado = resultado;
	equipo1Param->equipo_param=1;
	equipo2Param->equipo_param=2;
	equipo3Param->equipo_param=3;
		
	pthread_mutex_init(&salero,NULL);
	pthread_mutex_init(&horno,NULL);
	pthread_mutex_init(&plancha,NULL);

	//creo las variables de los hilos de los tres equipos
	pthread_t equipo1; 
	pthread_t equipo2;
	pthread_t equipo3;
 
	//inicializo los hilos de los tres equipos
    rc = pthread_create(&equipo1,                           //identificador unico
			 NULL,                          //atributos del thread
                         ejecutarReceta,             //funcion a ejecutar
                         equipo1Param); 

    rc = pthread_create(&equipo2,                           //identificador unico
                         NULL,                          //atributos del thread
                         ejecutarReceta,             //funcion a ejecutar
                         equipo2Param);

    rc = pthread_create(&equipo3,                           //identificador unico
                         NULL,                          //atributos del thread
                         ejecutarReceta,             //funcion a ejecutar
                         equipo3Param);

   if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     } 

	//join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
	pthread_join (equipo3,NULL);

	//muestra en pantalla al ganador
	printf("Y EL GANADOR ES EL EQUIPO %d \n", ganador);
	fprintf(resultado,"Y EL GANADOR ES EL EQUIPO %d \n", ganador);	
	
	fclose(resultado);
	
	pthread_mutex_destroy(&salero);
	pthread_mutex_destroy(&horno);
	pthread_mutex_destroy(&plancha);

    pthread_exit(NULL);
}



//Para compilar:   gcc HellsBurgers.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
