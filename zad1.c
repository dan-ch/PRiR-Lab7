#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define PORT 1
#define PORT_ROZLADOWANIE 2
#define PORT_LADOWANIE 3
#define START 4
#define REJS 5
#define KONIEC 6
#define KATASTROFA 7
#define TANKUJ 5000
#define REZERWA 500


int DOKUJ = 10;
int NIE_DOKUJ = 11;
int liczba_procesow;
int nr_procesu;
int ilosc_dokow;
int ilosc_portow;
int tag=1;
int ilosc_zajetych_dokow = 0;
MPI_Status mpi_status;

void Wyslij(int nr_statku, int stan, int kier)
{
    int wyslij[3];
    wyslij[0] = nr_statku;
    wyslij[1] = stan;
    wyslij[2] = kier;
    MPI_Send(&wyslij, 3, MPI_INT, kier, tag, MPI_COMM_WORLD);
    sleep(1);
}

void Port(int liczba_procesow)
{
    int nr_statku, status, kierunek, wariant;
    int odbierz[3];
    int ilosc_zajetych_dokow = 0;
    int ilosc_statkow = liczba_procesow - ilosc_portow;
    printf("Halo, Witam serdecznie z portu numer %d\n", nr_procesu);

    if(rand()%2==1)
    {
        printf("Mamy piekna pogode w porcie numer %d\n", nr_procesu);
    }
    else
    {
        printf("Niestety pogoda nie sprzyja dzisiejszym rejsom\n");
    }
        printf("Dysponujemy %d dokami \n", ilosc_dokow);
        sleep(2);
    while(ilosc_statkow > ilosc_dokow * ilosc_portow)
    {
        MPI_Recv(&odbierz,3,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
        nr_statku=odbierz[0];
        status=odbierz[1];
        kierunek=odbierz[2];
    if(status==PORT)
    {
        printf("Statek %d czeka w porcie numer %d na obsluge\n", nr_statku, nr_procesu);
    }
    else if(status==REJS)
    {
        printf("Statek %d widzimy cie na radarze port %d\n", nr_statku, nr_procesu);
    }
    else if(status==START)
    {
        printf("Statek %d czeka na wyplyniecie z portu numer %d\n", nr_statku, nr_procesu);
        ilosc_zajetych_dokow--;
    }
    else if(status==PORT_ROZLADOWANIE)
    {
        if(nr_statku%2==1){
            printf("Statek %d ludzie opuszczaja poklad\n", nr_statku);
        }
        else
            printf("Statek %d towray sa rozladowywane\n", nr_statku);
    }
    else if(status==PORT_LADOWANIE)
    {
        if(nr_statku%2==1){
            printf("Statek %d ludzie wchodza na poklad\n", nr_statku);
        }
        else
            printf("Statek %d towray sa ladowane \n", nr_statku);
    }
    else if(status==REJS)
    {
        printf("Statek %d jest w trakcie rejsu\n", nr_statku);
    }
    else if(status==KONIEC)
    {
        if(ilosc_zajetych_dokow<ilosc_dokow)
        {
            ilosc_zajetych_dokow++;
            MPI_Send(&DOKUJ, 1, MPI_INT, nr_statku, tag, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Send(&NIE_DOKUJ, 1, MPI_INT, nr_statku, tag, MPI_COMM_WORLD);
        }
    }
    if(status==KATASTROFA)
    {
        printf("Port %d otzrymał informacje o katastrofie\n", nr_procesu);
    }
    }
}

void Statek()
{
int  stan,suma,i;
stan=REJS;
int kierunek = rand()%ilosc_portow;
int paliwo = (rand()%2000 + 500);
//to chyba jedyny rozsadny stan z jakiego warto startowac
while(1){
if(stan==PORT){
    paliwo = TANKUJ;
    stan = PORT_ROZLADOWANIE;
    Wyslij(nr_procesu,stan, kierunek);
}
else if(stan == PORT_ROZLADOWANIE)
{
    stan = PORT_LADOWANIE;
    Wyslij(nr_procesu,stan, kierunek);
}
else if(stan == PORT_LADOWANIE)
{
    stan = START;
    Wyslij(nr_procesu,stan, kierunek);

}
else if(stan==START)
{
    kierunek = rand()%ilosc_portow;
    printf("Plyne, statek %d w kierunku portu numer %d \n",nr_procesu, kierunek);
    stan=REJS;
}
else if(stan==REJS)
{
    paliwo-= (rand()%200 + 200);
    Wyslij(nr_procesu,stan, kierunek);
    if(paliwo<=REZERWA)
    {
        stan=KONIEC;
        printf("Statek %d prosze o pozwolenie na wplyniecie do portu %d\n", nr_procesu, kierunek);
        Wyslij(nr_procesu,stan, kierunek);
    }
}
else if(stan == KATASTROFA){
    sleep(1);
    printf("Zbudowano nowy statek\n");
    stan = PORT_LADOWANIE;
    Wyslij(nr_procesu,stan, kierunek);

}
else if(stan==KONIEC)
{
    int temp;
    MPI_Recv(&temp, 1, MPI_INT, kierunek, tag, MPI_COMM_WORLD, &mpi_status);
    if(temp==DOKUJ)
    {
        stan=PORT;
        printf("Jestem w porcie, statek %d\n", nr_procesu);
        Wyslij(nr_procesu, stan, kierunek);
    }
    else
    {
        paliwo-=rand()%200;
        if(paliwo>0)
        {
            Wyslij(nr_procesu,stan, kierunek);
        }
        else
        {
            stan=KATASTROFA;
            printf("rozbilem sie statek numer %d\n", nr_procesu);
            for(int j=0; j<ilosc_portow; j++){
                Wyslij(nr_procesu,stan, j);
            }
        }
    }
}
}
}

int main(int argc, char *argv[])
{
    ilosc_portow = atoi(argv[1]);
    ilosc_dokow = atoi(argv[2]);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
    MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
    srand(time(NULL) + nr_procesu);
    if(nr_procesu >= 0 && nr_procesu < ilosc_portow)
    {
        Port(liczba_procesow);
    }
    else
        Statek();
MPI_Finalize();
printf("Koniec");
return 0;
}
