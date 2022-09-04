# OPEN PROCESS

Per la creazione di questo progetto ho sfruttato come base la distribuzione di **disastrOS** messa a disposizione dal professore sul suo repository, modificato a dovere il **makefile**
e aggiunto i file utili alla creazione delle system call da implementare

## COSA HO FATTO

Ho creato due system call all'interno dell'environment disastrOS, chiamate `disastrOS_exec` e `disastrOS_terminate`, che si occupano rispettivamente di:
- Trasformare un program file in un blocco di codice eseguibile in memoria (ad emulare il comportamento di una `exec`). Il puntatore alla funzione che 
  sarà eseguita viene recuperato tramite un simbolo cercato all'interno di un file `<XXX>.so`

- Terminare, come dice la segnatura, un thread su richiesta di un altro thread. Anche in questo caso viene rispettato il modus operandi della funzione
  `disastrOS_exit`, quindi vengono manipolate le strutture dei thread e le liste di processi in ready, waiting, running e zombie a dovere

## QUALE PROBLEMA RISOLVE

Come già detto, la funzione `disastrOS_exec` permette di caricare in memoria ed eseguire una funzione `<XXX>` che viene recuperata da un file `<XXX>.so`.
Il simbolo rappresentante la funzione e il file sono i due paramentri della funzione che vengono forniti dall'utente. Successivamente, tramite le syscall
`dlopen` e `dlsym`, viene recuperato il puntatore alla funzione che verrà eseguita da un thread generato appositamente. La creazione del nuovo thread
rispetta i principi visti all'interno di `disastrOS_spawn`

Per quanto riguarda `disastrOS_terminate` invece, dà la possibilità ad un thread di richiedere la terminazione di un altro thread, andandolo prima ad
inserire nella lista di thread Zombie e successivamente a eliminarlo completamente dalla memoria, come accade in `disastrOS_exit`

## COME FARLO GIRARE

Portarsi all'interno della directory `disastrOS_04_resources` ed inserire nel terminale il seguente comando:

```
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PWD}
```

Questo aggiunge alla evn **LD_LIBRARY_PATH** la directory corrente alla lista di directory nelle quali il sistema va a cercare le librerie dinamiche da linkare al programma

A questo punto basta compilare tramite

```
make
```

e il compilatore genererà tutti i file `<XXX>.o` necessari a creare la libreria dinamica utile a far girare il programma.

Infine per far partire il programma eseguire il comando

```
./disastrOS_test
```