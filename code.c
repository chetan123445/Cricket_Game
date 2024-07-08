#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>


struct node
{
    int run;
    char A;
    char B;
    struct node *link;
};
struct node *head=NULL;
struct node *temp;

int runs=0;
int runs2=0;

struct teamdetails
{
    char team[15];
    int matches;
    int matches_won;
    int matches_lost;
    int matches_draw;
    int points;
    int decode_number;
};


void add_in_linked_list(int run,char A,char B)
{
    struct node *newnode=(struct node *)malloc(sizeof(struct node));
    newnode->run=run;
    newnode->A=A;
    newnode->B=B;
    newnode->link=NULL;
    if(head==NULL)
    {
        head=newnode;
    }
    else
    {
        temp->link=newnode;
    }
    temp=newnode;
}


void show_linked_list()
{
    temp=head;
    while(temp!=NULL)
    {
        printf("|%d",temp->run);
        printf("%c",temp->A);
        printf("%c|  ",temp->B);
        temp=temp->link;
    }
    printf("\n");
    head=NULL;
}


void Take_team_name(int total_teams,struct teamdetails group1[],struct teamdetails group2[])
{
    
    //initialise team details of group 1 before the tournament start
    printf("Enter the team names of group1:\n");
    for(int i=0;i<(total_teams/2);i++)
    {
        printf("Team %d:",i+1);
        scanf("%s",group1[i].team);
        group1[i].matches=0;
        group1[i].matches_won=0;
        group1[i].matches_lost=0;
        group1[i].matches_draw=0;
        group1[i].points=0;
        group1[i].decode_number=i;
    }
    
    printf("\n");
    
    
    //initialise team details of group 2 before the tournament start
    printf("Enter the team names of group2:\n");
    for(int i=0;i<(total_teams/2);i++)
    {
        printf("Team %d:",i+1);
        scanf("%s",group2[i].team);
        group2[i].matches=0;
        group2[i].matches_won=0;
        group2[i].matches_lost=0;
        group2[i].matches_draw=0;
        group2[i].points=0;
        group2[i].decode_number=i;
    }
    
    printf("\n");
}


void Print_details(int total_teams,struct teamdetails group1[],struct teamdetails group2[])
{
    int choice;
    do
    {
        printf("For Group1 details(Enter 1) / Group2 details(Enter 2)/Exit(Enter 3):");
        scanf("%d",&choice);
        if(choice==1)
        {
           printf("\n                       GROUP 1\n");
           printf("                       ======\n");
           printf("Team No.|    Team      |      Won    |   Lost    |  Draw   |   Points  |   Matches |\n");
           printf("------------------------------------------------------------------------------------\n");
           for(int i=0;i<(total_teams/2);i++)
           {
               int len=strlen(group1[i].team);
                printf("%d:",i+1);
                for(int j=0;j<10;j++)
                {
                    if(j==6)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%s",group1[i].team);
                for(int j=0;j<19-len;j++)
                {
                    if(j==11-len)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group1[i].matches_won);
                for(int j=0;j<10;j++)
                {
                    if(j==5)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group1[i].matches_lost);
                for(int j=0;j<10;j++)
                {
                    if(j==6)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group1[i].matches_draw);
                for(int j=0;j<12;j++)
                {
                   if(j==5)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group1[i].points);
                for(int j=0;j<12;j++)
                {
                   if(j==4)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group1[i].matches);
                printf("   |\n");
                printf("------------------------------------------------------------------------------------\n");
           }
           printf("\n");
        }
        else if(choice==2)
        {
           printf("\n                       GROUP 2\n");
           printf("                       ======\n");
           printf("Team No.|    Team      |      Won    |   Lost    |  Draw   |   Points  |   Matches |\n");
           printf("------------------------------------------------------------------------------------\n");
           for(int i=0;i<(total_teams/2);i++)
           {
               int len=strlen(group2[i].team);
                printf("%d:",i+1);
                for(int j=0;j<10;j++)
                {
                    if(j==6)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%s",group2[i].team);
                for(int j=0;j<19-len;j++)
                {
                    if(j==11-len)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group2[i].matches_won);
                for(int j=0;j<10;j++)
                {
                    if(j==5)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group2[i].matches_lost);
                for(int j=0;j<10;j++)
                {
                    if(j==6)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group2[i].matches_draw);
                for(int j=0;j<12;j++)
                {
                   if(j==5)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group2[i].points);
                for(int j=0;j<12;j++)
                {
                   if(j==4)
                    {
                        printf("|");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                printf("%d",group2[i].matches);
                printf("   |\n");
                printf("------------------------------------------------------------------------------------\n");
           }
           printf("\n");
        }
    }while(choice!=3);
}


int Total_Matches(int total_teams)
{
    int i=1;
    int matches=0;
    while(((total_teams/2)-i)>0)
    {
        matches=matches+((total_teams/2)-i);
        i++;
    }
    return 2*matches;
}


bool Can_happen(int total_teams,int random_team1,int random_team2,int myteamdecodenumber,int matches_done_group[][total_teams/2],int choose_random_group,int myteamgroupnumber)
{
    if(myteamgroupnumber==choose_random_group)
    {
        if(random_team1==myteamdecodenumber)
        {
            return false;
        }
        else if(random_team2==myteamdecodenumber)
        {
            return false;
        }
    }
    if(matches_done_group[random_team1][random_team2]==1)
    {
        return false;
    }
    if(matches_done_group[random_team1][random_team2]==0)
    {
        matches_done_group[random_team1][random_team2]=1;
        matches_done_group[random_team2][random_team1]=1;
        return true;
    }
}



void Match_organisation(int total_teams,int matches_done_group[][total_teams/2],int myteamdecodenumber,int myteamgroupnumber,struct teamdetails group[],int choose_random_group)
{
    int random_team1;
    int random_team2;
    bool x=false;
    while(x!=true)
    {
        random_team1=rand()%(total_teams/2);
        random_team2=rand()%(total_teams/2);
            x=Can_happen(total_teams,group[random_team1].decode_number,group[random_team2].decode_number,myteamdecodenumber,matches_done_group,choose_random_group,myteamgroupnumber);
    }
    char team1[15];
    char team2[15];
    strcpy(team1,group[random_team1].team);
    strcpy(team2,group[random_team2].team);
    
    printf("\nThe match was between %s and %s\n\n",team1,team2);
    
    int result=rand()%3;
    int winner=rand()%2;
    if(result==0)//won
    {
        if(winner==0)
        {
            group[random_team1].matches=group[random_team1].matches+1;
            group[random_team1].matches_won=group[random_team1].matches_won+1;
            group[random_team1].points=group[random_team1].points+2;
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_lost=group[random_team2].matches_lost+1;
        }
        else
        {
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_won=group[random_team2].matches_won+1;
            group[random_team2].points=group[random_team2].points+2;
            group[random_team1].matches=group[random_team1].matches+1;
            group[random_team1].matches_lost=group[random_team1].matches_lost+1;
        }
    }
    else if(result==1)//lost
    {
        if(winner==0)
        {
            group[random_team1].matches=group[random_team1].matches+1;
            group[random_team1].matches_lost=group[random_team1].matches_lost+1;
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_won=group[random_team2].matches_won+1;
            group[random_team2].points=group[random_team2].points+2;
            
        }
        else
        {
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_lost=group[random_team2].matches_lost+1;
            group[random_team1].matches=group[random_team1].matches+1;
            group[random_team1].matches_won=group[random_team1].matches_won+1;
            group[random_team1].points=group[random_team1].points+2;
        }
    }
    else//draw
    {
            group[random_team1].matches=group[random_team1].matches+1;
            group[random_team1].matches_draw=group[random_team1].matches_draw+1;
            group[random_team1].points=group[random_team1].points+1;
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_draw=group[random_team2].matches_draw+1;
            group[random_team2].points=group[random_team2].points+1;
    }
}


void Other_teams_matches(int total_teams,int choose_random_group,int matches_done_group1[][total_teams/2],int matches_done_group2[][total_teams/2],int myteamlen,char myteam[myteamlen],int myteamdecodenumber,int myteamgroupnumber,struct teamdetails group1[],struct teamdetails group2[])
{
    if(choose_random_group==1)
    {
         Match_organisation(total_teams,matches_done_group1,myteamdecodenumber,myteamgroupnumber,group1,choose_random_group);
    }
    else
    {
        Match_organisation(total_teams,matches_done_group2,myteamdecodenumber,myteamgroupnumber,group2,choose_random_group);
    }
}


void Autoplay_first_inning(int over,int maxovers,int run_rate,int wickets)
{
    char A;
    char B;
    int runs_on_that_ball;
    int no_ball;
    char array[11]={'O','T','w','N','B','F','t','L','W','D','S'};
    int random;
    //O=one run, T=two runs, w=wide, N=no ball, B=bye, F=Four, t=Threes, L=leg Bye,W=wicket, D=Dot ball, S=six, 
    int i,x,j;
    for(i=over;i<maxovers;i++)
    {
        printf("\n                              OVER %d:\n",i+1);
        for(j=0;j<6;j++)
        {
            if(wickets==10)
            {
                break;
            }
            random=rand()%11;
            if(random==0)
            {
                  runs=runs+1;
                  runs_on_that_ball=1;
                  A=' ';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==1)
            {
                runs=runs+2;
                runs_on_that_ball=2;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==2)
            {
                runs=runs+1;
                runs_on_that_ball=1;
                 A='w';
                 B=' ';
                 add_in_linked_list(runs_on_that_ball,A,B);
                j--;
            }
            else if(random==3)
            {
                x=rand()%7;
                runs=runs+x+1;
                runs_on_that_ball=x+1;
                A='N';
                B='B';
                no_ball=j;
                add_in_linked_list(runs_on_that_ball,A,B);
                j--;
            }
            else if(random==4)
            {
                x=rand()%5;
                runs=runs+x;
                runs_on_that_ball=x;
                A='B';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==5)
            {
                runs=runs+4;
                runs_on_that_ball=4;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==6)
            {
                runs=runs+3;
                runs_on_that_ball=3;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==7)
            {
                x=rand()%5;
                runs=runs+x;
                runs_on_that_ball=x;
                A='L';
                B='B';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            if(random==8)
           {
              if(j!=no_ball)
              {
                  wickets++; 
                  runs_on_that_ball=0;
                  A='W';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
              else
              {
                  int how_many_runs;
                  how_many_runs=rand()%7;
                  runs=runs+how_many_runs;
                  runs_on_that_ball=how_many_runs;
                  A=' ';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
           }
            else if(random==9)
            {
                runs_on_that_ball=0;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==10)
            {
                runs=runs+6;
                runs_on_that_ball=6;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
        }
        if(wickets==10)
        {
            show_linked_list();
            if(j>=5)
            {
               printf("\n\nScore:%d-%d                           Overs:%d            Run Rate:%d\n\n",runs,wickets,i+1,runs/(i+1)); 
            }
            else
            {
                printf("\n\nScore:%d-%d                           Overs:%d.%d            Run Rate:%d\n\n",runs,wickets,i,j,runs/(i+1));
            }
            break;
        }
         no_ball=-1;
         printf("\nOver %d: ",i+1);
         show_linked_list();
         printf("\nScore:%d-%d           Overs:%d            Run Rate:%d\n",runs,wickets,i+1,runs/(i+1));
    }
    printf("\n                             First Innings Complete\n");
    printf("                             ========================\n");
}


void Autoplay_second_inning(int over,int maxovers,int run_rate,int wickets)
{
    char A;
    char B;
    int runs_on_that_ball;
    int no_ball;
    char array[11]={'O','T','w','N','B','F','t','L','W','D','S'};
    int random;
    //O=one run, T=two runs, w=wide, N=no ball, B=bye, F=Four, t=Threes, L=leg Bye,W=wicket, D=Dot ball, S=six, 
  int i,j;
    for(i=over;i<maxovers;i++)
    {
        printf("\n                              OVER %d:\n",i+1);
        for(j=0;j<6;j++)
        {
            if(runs2>runs || wickets==10)
            {
                break;
            }
            random=rand()%11;
            if(random==0)
            {
                  runs2=runs2+1;
                  runs_on_that_ball=1;
                  A=' ';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==1)
            {
                runs2=runs2+2;
                runs_on_that_ball=2;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==2)
            {
                runs2=runs2+1;
                runs_on_that_ball=1;
                 A='w';
                 B=' ';
                 add_in_linked_list(runs_on_that_ball,A,B);
                j--;
            }
            else if(random==3)
            {
                int x=rand()%7;
                runs2=runs2+x+1;
                runs_on_that_ball=x+1;
                A='N';
                B='B';
                add_in_linked_list(runs_on_that_ball,A,B);
                j--;
            }
            else if(random==4)
            {
                int x=rand()%5;
                runs2=runs2+x;
                runs_on_that_ball=x;
                A='B';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==5)
            {
                runs2=runs2+4;
                runs_on_that_ball=4;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==6)
            {
                runs2=runs2+3;
                runs_on_that_ball=3;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==7)
            {
                int x=rand()%5;
                runs2=runs2+x;
                runs_on_that_ball=x;
                A='L';
                B='B';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            if(random==8)
           {
              if(j!=no_ball)
              {
                  wickets++; 
                  runs_on_that_ball=0;
                  A='W';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
              else
              {
                  int how_many_runs;
                  how_many_runs=rand()%7;
                  runs2=runs2+how_many_runs;
                  runs_on_that_ball=how_many_runs;
                  A=' ';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
           }
            else if(random==9)
            {
                runs_on_that_ball=0;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            else if(random==10)
            {
                runs2=runs2+6;
                runs_on_that_ball=6;
                A=' ';
                B=' ';
                add_in_linked_list(runs_on_that_ball,A,B);
            }
            no_ball=-1;
        }
        if(runs2>runs || wickets==10)
        {
            show_linked_list();
            if(j>=5)
            {
               printf("\n\nScore:%d-%d                           Overs:%d            Run Rate:%d\n\n",runs2,wickets,i+1,runs2/(i+1)); 
            }
            else
            {
                printf("\n\nScore:%d-%d                           Overs:%d.%d            Run Rate:%d\n\n",runs2,wickets,i,j,runs2/(i+1));
            }
            break;
        }
         printf("\nOver %d: ",i+1);
         show_linked_list();
         printf("\nScore:%d-%d                           Overs:%d            Run Rate:%d\n",runs2,wickets,i+1,runs2/(i+1));
         if(runs2>runs)
         {
             break;
         }
    }
    printf("\n                             Second Innings Complete\n");
    printf("                             ========================\n");
}


bool Can_happen_mymatch(int total_teams,int random_team2,int myteamdecodenumber,int matches_done_group[][total_teams/2])
{
        if(random_team2==myteamdecodenumber)
        {
            return false;
        }
        if(matches_done_group[random_team2][myteamdecodenumber]==1)
        {
            return false;
        }
        if(matches_done_group[random_team2][myteamdecodenumber]==0)
        {
            matches_done_group[random_team2][myteamdecodenumber]=1;
            matches_done_group[myteamdecodenumber][random_team2]=1;
            return true;
        }
}



void Match_organisation_myteam(int total_teams,struct teamdetails group[],int matches_done_group[][total_teams/2],int myteamdecodenumber,int myteamgroupnumber,char myteam[],char who_bat_first)
{
    int random_team2;
    bool x=false;
    while(x!=true)
    {
        random_team2=rand()%(total_teams/2);
        x=Can_happen_mymatch(total_teams,group[random_team2].decode_number,myteamdecodenumber,matches_done_group);
    }
    char team2[15];
    strcpy(team2,group[random_team2].team);
        //result of the match    
    if(runs>runs2)
    {
        if(who_bat_first=='I')
        {
            group[myteamdecodenumber].matches=group[myteamdecodenumber].matches+1;
            group[myteamdecodenumber].matches_won=group[myteamdecodenumber].matches_won+1;
            group[myteamdecodenumber].points=group[myteamdecodenumber].points+2;
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_lost=group[random_team2].matches_lost+1;
        }
        else
        {
            group[myteamdecodenumber].matches=group[myteamdecodenumber].matches+1;
            group[myteamdecodenumber].matches_lost=group[myteamdecodenumber].matches_lost+1;
            group[random_team2].points=group[random_team2].points+2;
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_won=group[random_team2].matches_won+1;
        }
    }
    else if(runs==runs2)
    {
        group[myteamdecodenumber].matches=group[myteamdecodenumber].matches+1;
        group[myteamdecodenumber].matches_draw=group[myteamdecodenumber].matches_draw+1;
        group[myteamdecodenumber].points=group[myteamdecodenumber].points+1;
        group[random_team2].matches=group[random_team2].matches+1;
        group[random_team2].matches_draw=group[random_team2].matches_draw+1;
        group[random_team2].points=group[random_team2].points+1;   
    }
    else
    {
        if(who_bat_first=='I')
        {
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_won=group[random_team2].matches_won+1;
            group[random_team2].points=group[random_team2].points+2;
            group[myteamdecodenumber].matches=group[myteamdecodenumber].matches+1;
            group[myteamdecodenumber].matches_lost=group[myteamdecodenumber].matches_lost+1;
        }
        else
        {
            group[random_team2].matches=group[random_team2].matches+1;
            group[random_team2].matches_lost=group[random_team2].matches_lost+1;
            group[myteamdecodenumber].points=group[myteamdecodenumber].points+2;
            group[myteamdecodenumber].matches=group[myteamdecodenumber].matches+1;
            group[myteamdecodenumber].matches_won=group[myteamdecodenumber].matches_won+1;
        }
    }
    printf("\nThe match was between %s and %s\n\n",myteam,team2);
}



void Mine_team_matches(int total_teams,struct teamdetails group[],int matches_done_group[][total_teams/2],int myteamdecodenumber,int myteamgroupnumber,char myteam[])
{
    printf("\nStrict Rule:You have to play atleast one over\n");
    printf("How many overs game do you want to play:");
    int maxovers;
    scanf("%d",&maxovers);
    printf("\n                          Let's Do the toss\n");
    printf("                          ==================\n");
    int whos_call=rand()%2;
    char toss_call;
    int oppo_decide;
    int you_decide;
    char who_bat_first;
    if(whos_call==1)
    {
        printf("Yours Call(Head-H / Tail-T):");
        getchar();
        scanf("%c",&toss_call);
        int outcome=rand()%2;
        if(outcome==0 && toss_call=='H')
        {
            printf("You won the toss\n");
            printf("What you are going to do(BAT-1 / Field-0):");
            scanf("%d",&you_decide);
            if(you_decide==0)
            {
                who_bat_first='O';
            }
            else
            {
                who_bat_first='I';
            }
        }
        else if(outcome==0 && toss_call=='T')
        {
            printf("You lost the toss\n");
            oppo_decide=rand()%2;
            if(oppo_decide==0)
            {
                printf("Opponent has decided to FIELD first\n");
                who_bat_first='I';
            }
            else
            {
                printf("Opponent has decided to BAT first\n");
                who_bat_first='O';
            }
        }
        else if(outcome==1 && toss_call=='T')
        {
            printf("You won the toss\n");
            printf("What you are going to do(BAT-1 / Field-0):");
            scanf("%d",&you_decide);
            if(you_decide==0)
            {
                who_bat_first='O';
            }
            else
            {
                who_bat_first='I';
            }
        }
        else if(outcome==1 && toss_call=='H')
        {
            printf("You lost the toss\n");
            oppo_decide=rand()%2;
            if(oppo_decide==0)
            {
                printf("Opponent has decided to FIELD first\n");
                who_bat_first='I';
            }
            else
            {
                printf("Opponent has decided to BAT first\n");
                who_bat_first='O';
            }
        }
    }
    else
    {
        printf("Opponent has won the toss\n");
        oppo_decide=rand()%2;
        if(oppo_decide==0)
        {
            printf("Opponent has decided to FIELD first\n");
            who_bat_first='I';
        }
        else
        {
            printf("Opponent has decided to BAT first\n");
            who_bat_first='O';
        }
    }
    
    printf("\n                            First Inning Start\n");
    printf("                            ==================\n\n");
    int autoplay_choice;
    int run_rate;
    int max_wickets=10;
    int wickets=0;
    
    char event_happen;
    char A;
    char B;
    int runs_on_that_ball;
    int no_ball;
    int i,j;
    for(i=0;i<maxovers;i++)
    {
        printf("Enter event on the ball(Runs-R / Wicket- W / Wide-w / Bye-B / Leg Bye-L / Dot ball-D / No Ball-N)\n");
        printf("                              OVER %d:\n",i+1);
        for(j=0;j<6;j++)
        {
            if(wickets==10)
            {
                break;
            }
           printf("\nEnter event:");    
           scanf(" %c",&event_happen);
           if(event_happen=='W')
           {
              if(j!=no_ball)
              {
                  wickets++; 
                  runs_on_that_ball=0;
                  A='W';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
              else
              {
                  int how_many_runs;
                  printf("How many runs on that ball:");
                  scanf("%d",&how_many_runs);
                  runs=runs+how_many_runs;
                  runs_on_that_ball=how_many_runs;
                  A=' ';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
           }
           else if(event_happen=='R')
           {
               int how_many_runs;
               printf("How many runs on that ball:");
               scanf("%d",&how_many_runs);
               runs=runs+how_many_runs;
               runs_on_that_ball=how_many_runs;
               A=' ';
               B=' ';
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else if(event_happen=='w')//wide
           {
               runs=runs+1;
               runs_on_that_ball=1;
               j--;
               A='w';
               B=' ';
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else if(event_happen=='B'  ||  event_happen=='L')//bye and leg bye
           {
               int how_many_runs;
               printf("How many runs on that bye:");
               scanf("%d",&how_many_runs);
               runs=runs+how_many_runs;
               runs_on_that_ball=how_many_runs;
               if(event_happen=='B')
               {
                   A='B';
                   B=' ';
               }
               else
               {
                   A='L';
                   B='B';
               }
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else if(event_happen=='D')
           {
               runs_on_that_ball=0;
               A=' ';
               B=' ';
               add_in_linked_list(runs_on_that_ball,A,B);
               
           }
           else if(event_happen=='N')
           {
               no_ball=j;
               runs=runs+1;
               j--;
               int how_many_runs_on_no_ball;
               printf("How many runs on that no ball:");
               scanf("%d",&how_many_runs_on_no_ball);
               runs=runs+how_many_runs_on_no_ball;
               runs_on_that_ball=how_many_runs_on_no_ball;
               A='N';
               B='B';
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else
           {
               printf("Invalid event");
               j--;
           }
        }
        if(wickets==10)
        {
            show_linked_list();
            if(j>=5)
            {
               printf("\n\nScore:%d-%d                           Overs:%d            Run Rate:%d\n\n",runs,wickets,i+1,runs/(i+1)); 
            }
            else
            {
                printf("\n\nScore:%d-%d                           Overs:%d.%d            Run Rate:%d\n\n",runs,wickets,i,j,runs/(i+1));
            }
            break;
        }
        no_ball=-1;
        printf("\nOver %d: ",i+1);
        show_linked_list();
        printf("\n\nScore:%d-%d                           Overs:%d            Run Rate:%d\n\n",runs,wickets,i+1,runs/(i+1));
        printf("Do you want to autoplay(Yes-1 / No-0):");
        scanf("%d",&autoplay_choice);
        if(autoplay_choice==1)
        {
            Autoplay_first_inning(i+1,maxovers,run_rate,wickets);
            break;
        }
    }
    printf("\n                            Second Inning Starts\n");
    printf("                            ====================\n");
    
    run_rate;
    max_wickets=10;
    wickets=0;
    
     for(i=0;i<maxovers;i++)
    {
        printf("Enter event on the ball(Runs-R / Wicket- W / Wide-w / Bye-B / Leg Bye-L / Dot ball-D / No Ball-N)\n");
        printf("                              OVER %d:\n",i+1);
        for(j=0;j<6;j++)
        {
            if(runs2>runs || wickets==10)
            {
                break;
            }
           printf("\nEnter event:");    
           scanf(" %c",&event_happen);
           if(event_happen=='W')
           {
              if(j!=no_ball)
              {
                  wickets++; 
                  runs_on_that_ball=0;
                  A='W';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
              else
              {
                  int how_many_runs;
                  printf("How many runs on that ball:");
                  scanf("%d",&how_many_runs);
                  runs2=runs2+how_many_runs;
                  runs_on_that_ball=how_many_runs;
                  A=' ';
                  B=' ';
                  add_in_linked_list(runs_on_that_ball,A,B);
              }
           }
           else if(event_happen=='R')
           {
               int how_many_runs;
               printf("How many runs on that ball:");
               scanf("%d",&how_many_runs);
               runs2=runs2+how_many_runs;
               runs_on_that_ball=how_many_runs;
               A=' ';
               B=' ';
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else if(event_happen=='w')//wide
           {
               runs2=runs2+1;
               runs_on_that_ball=1;
               j--;
               A='w';
               B=' ';
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else if(event_happen=='B'  ||  event_happen=='L')//bye and leg bye
           {
               int how_many_runs;
               printf("How many runs on that bye:");
               scanf("%d",&how_many_runs);
               runs2=runs2+how_many_runs;
               runs_on_that_ball=how_many_runs;
               if(event_happen=='B')
               {
                   A='B';
                   B=' ';
               }
               else
               {
                   A='L';
                   B='B';
               }
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else if(event_happen=='D')
           {
               runs_on_that_ball=0;
               A=' ';
               B=' ';
               add_in_linked_list(runs_on_that_ball,A,B);
               
           }
           else if(event_happen=='N')
           {
               no_ball=j;
               runs2=runs2+1;
               j--;
               int how_many_runs_on_no_ball;
               printf("How many runs on that no ball:");
               scanf("%d",&how_many_runs_on_no_ball);
               runs2=runs2+how_many_runs_on_no_ball;
               runs_on_that_ball=how_many_runs_on_no_ball;
               A='N';
               B='B';
               add_in_linked_list(runs_on_that_ball,A,B);
           }
           else
           {
               printf("Invalid event");
               j--;
           }
        }
        if(runs2>runs || wickets==10)
        {
            show_linked_list();
            if(j>=5)
            {
               printf("\n\nScore:%d-%d                           Overs:%d            Run Rate:%d\n\n",runs2,wickets,i+1,runs2/(i+1)); 
            }
            else
            {
                printf("\n\nScore:%d-%d                           Overs:%d.%d            Run Rate:%d\n\n",runs2,wickets,i,j,runs2/(i+1));
            }
            break;
        }
        no_ball=-1;
        printf("\nOver %d: ",i+1);
        show_linked_list();
        printf("\n\nScore:%d-%d                           Overs:%d            Run Rate:%d\n\n",runs2,wickets,i+1,runs2/(i+1));
        printf("Do you want to autoplay(Yes-1 / No-0):");
        scanf("%d",&autoplay_choice);
        if(autoplay_choice==1)
        {
            Autoplay_second_inning(i+1,maxovers,run_rate,wickets);
            break;
        }
    }
    Match_organisation_myteam(total_teams,group,matches_done_group,myteamdecodenumber,myteamgroupnumber,myteam,who_bat_first);
}


void Sorted_order(int total_teams,struct teamdetails group[])
{
    struct teamdetails temp;
    for(int i=0;i<total_teams/2-1;i++)
    {
        for(int j=0;j<total_teams/2-i-1;j++)
        {
            if(group[j].points<group[j+1].points)
            {
                temp=group[j+1];
                group[j+1]=group[j];
                group[j]=temp;
            }
        }
    }
}


int main()
{
    printf("               Let's Play the Cricket Tournament\n");
    printf("               =================================\n\n");
    int total_teams;
    printf("How many teams are there in the tournament(In even number):");
    scanf("%d",&total_teams);
    
    printf("\n");
    
    struct teamdetails group1[total_teams/2];
    struct teamdetails group2[total_teams/2];
    
    //Function for taking teams names as input
    Take_team_name(total_teams,group1,group2);
    
    //Function to print team details
    Print_details(total_teams,group1,group2);
    
    printf("\nChoose your team from these %d teams:",total_teams);
    char myteam[15];
    scanf("%s",myteam);
    printf("Your team is %s\n",myteam);
    
    
    int myteamlen=strlen(myteam);
    int myteamgroupnumber=-1;
    int myteamdecodenumber;
    for(int i=0;i<(total_teams/2);i++)
    {
       if(strcmp(myteam,group1[i].team)==0)
       {
           myteamgroupnumber=1;
           myteamdecodenumber=i;
           break;
       }
    }
    if(myteamgroupnumber==-1)
    {
       for(int i=0;i<(total_teams/2);i++)
       {
           if(strcmp(myteam,group2[i].team)==0)
           {
               myteamgroupnumber=2;
               myteamdecodenumber=i;
               break;
           }
        }
    }
    
    int total_matches=Total_Matches(total_teams);
    int copy_total_matches=total_matches;
    int matches_in_each_group=total_matches/2;
    int matches_in_group1=0;
    int matches_in_group2=0;
    int mine_matches=0;
    int matches_in_each_group_except=(total_matches/2)-((total_teams/2)-1);//except your team
    
    //arrays to check that the match has been done between these teams or not
    int matches_done_group1[total_teams/2][total_teams/2];
    int matches_done_group2[total_teams/2][total_teams/2];
    
    for(int i=0;i<total_teams/2;i++)
    {
        for(int j=0;j<total_teams/2;j++)
        {
            if(i==j)
            {
              matches_done_group1[i][j]=1; 
              matches_done_group2[i][j]=1;
            }
            else
            {
                matches_done_group1[i][j]=0;
                matches_done_group1[j][i]=0;
                matches_done_group2[i][j]=0;
                matches_done_group2[j][i]=0; 
            }
        }
    }
    
    int choice;
    int choose_random_group;
    do
    {
       printf("\nWhat do you want to do:\n1:For team details(Enter 1)\n2:For Play your team's match(Enter 2)\n3:For Skip Other's team match(Enter 3)\n4:To exit(Enter 4)\n");
       printf("\nEnter your choice:");
       scanf("%d",&choice);
       if(choice==1)
       {
              //Function to print team details
              Print_details(total_teams,group1,group2);
       }
       else if(choice==2)
       {
           runs=0;
           runs2=0;
           if(mine_matches==total_teams/2-1)
           {
               printf("\nMy teams all matches has been done\n");
           }
           else
           {
               if(myteamgroupnumber==1)
               {
                    Mine_team_matches(total_teams,group1,matches_done_group1,myteamdecodenumber,myteamgroupnumber,myteam);
                    Sorted_order(total_teams,group1);
               }
               else
               {
                   Mine_team_matches(total_teams,group2,matches_done_group2,myteamdecodenumber,myteamgroupnumber,myteam);
                   Sorted_order(total_teams,group2);
               }
               mine_matches++;
               Print_details(total_teams,group1,group2);
               copy_total_matches--;
           }
       }
       else if(choice==3)
       {
           runs=0;
           runs2=0;
           if(matches_in_group1==matches_in_each_group_except && matches_in_group2==matches_in_each_group)
           {
               if(myteamgroupnumber==1)
               {
                    printf("\nNow only your teams matches are left\n");
               }
           }
           else if(matches_in_group2==matches_in_each_group_except && matches_in_group1==matches_in_each_group)
           {
               if(myteamgroupnumber==2)
               {
                    printf("\nNow only your teams matches are left\n");
               }
           }
           else
           {
               choose_random_group=rand()%2;
               choose_random_group=choose_random_group+1;
               if(choose_random_group==1 && myteamgroupnumber==choose_random_group && matches_in_group1==matches_in_each_group_except)
               {
                  choose_random_group=2; 
               }
               else if(choose_random_group==2 && myteamgroupnumber==choose_random_group && matches_in_group2==matches_in_each_group_except)
               {
                  choose_random_group=1;  
               }
               printf("\nRandomly chose group is %d\n",choose_random_group);
               if(choose_random_group==1)//for group 1
               {
                   Other_teams_matches(total_teams,choose_random_group,matches_done_group1,matches_done_group2,myteamlen,myteam,myteamdecodenumber,myteamgroupnumber,group1,group2);
                   matches_in_group1++;
                   Sorted_order(total_teams,group1);
               }
               else// for group 2
               {
                   Other_teams_matches(total_teams,choose_random_group,matches_done_group1,matches_done_group2,myteamlen,myteam,myteamdecodenumber,myteamgroupnumber,group1,group2);
                   matches_in_group2++;
                   Sorted_order(total_teams,group2);
               }
               Print_details(total_teams,group1,group2);
               copy_total_matches--;
           }
       }
       else if(choice==4)
       {
           break;
       }
       else
       {
           printf("Invalid choice\n");
       }
    }while(copy_total_matches>0);
    
    printf("                       TOURNAMENT ENDS\n");
    printf("                       ===============\n");
       
    return 0;
}
