//Handle multiple socket connections with select and fd_set on Linux  
#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include "server.h"

#define TRUE  1
#define FALSE 0

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

struct prio_queue sell_pq[11], buy_pq[11];
int logged_in[6];
struct trades trade_array[6];

void init(){
	for(int i=1;i<=10;i++)
		initialise(&sell_pq[i]);
	for(int i=1;i<=10;i++)
		initialise(&buy_pq[i]);
	for(int i=1;i<=5;i++)
		logged_in[i]=-1;
	for(int i=1;i<=5;i++)
		initialise_t(&trade_array[i]);
}

//-1 indicates no such user exists
//-2 indicates wrong password
//-3 indicates already logged in from somewhere else
//0 indicates authentication successful
int authenticate(char username[], char password[], int username_len, int password_len){
	FILE *fptr;
	fptr = fopen("traders_auth.txt","r");
	for(int i=1;i<=5;i++){
		char uname[2049], pwd[2049];
		bzero(uname,2049);
		bzero(pwd,2049);
		char c;
		int uname_len=0, pwd_len=0;
		while((c = fgetc(fptr))!=' '){
			uname[uname_len]=c;
			uname_len++;
		}
		while((c = fgetc(fptr))!='\n'){
			pwd[pwd_len]=c;
			pwd_len++;
		}
		if(uname_len!=username_len)
			continue;
		else{
			int umatch=1;
			for(int j=0;j<2049;j++){
				if(username[j]!=uname[j]){
					umatch=0;
					break;
				}
			}
			if(umatch==0){
				continue;
			}
			else{
				if(password_len!=pwd_len){
					fclose(fptr);
					return -2;
				}
				else{
					int pmatch=1;
					for(int j=0;j<2049;j++){
						if(password[j]!=pwd[j]){
							pmatch=0;
							break;
						}
					}
					if(pmatch==1){
						fclose(fptr);
						if(logged_in[i]==-1){
							return i;
						}
						else{
							return -3;
						}
					}
					else{
						fclose(fptr);
						return -2;
					}
				}
			}
		}
	}
	fclose(fptr);
	return -1;
}

void buy(int trader_id, int item_no, int quantity, int price){
	while(quantity>0&&((!isEmpty(&sell_pq[item_no]))&&min_price_min_heap(&sell_pq[item_no])<=price)){
		int seller_has = quantity_of_min_price_min_heap(&sell_pq[item_no]);
		if(seller_has>quantity){
			struct request mini;
			showMin_min_heap(&sell_pq[item_no], &mini);
			struct trade tr;
			tr.item_id = item_no;
			tr.seller_id = mini.trader_id;
			tr.buyer_id = trader_id;
			tr.price = mini.price;
			tr.quantity = quantity;

			insertIntoArray(tr, &trade_array[tr.seller_id]);
			insertIntoArray(tr, &trade_array[tr.buyer_id]);

			quantity -= tr.quantity;
			changeMinQuantity_min_heap(&sell_pq[item_no], mini.quantity - tr.quantity);
		}
		else{
			struct request mini;
			extractMin(&sell_pq[item_no], &mini);
			struct trade tr;
			tr.item_id = item_no;
			tr.seller_id = mini.trader_id;
			tr.buyer_id = trader_id;
			tr.price = mini.price;
			tr.quantity = mini.quantity;

			insertIntoArray(tr, &trade_array[tr.seller_id]);
			insertIntoArray(tr, &trade_array[tr.buyer_id]);

			quantity -= tr.quantity;
		}
	}
	if(quantity){
		struct request temp;
		temp.trader_id = trader_id;
		temp.price = price;
		temp.quantity = quantity;
		insertIntoMaxHeap(temp, &buy_pq[item_no]);
	}
	return;
}

void sell(int trader_id, int item_no, int quantity, int price){
	while(quantity>0&&((!isEmpty(&buy_pq[item_no]))&&max_price_max_heap(&buy_pq[item_no])>=price)){
		int buyer_has = quantity_of_max_price_max_heap(&buy_pq[item_no]);
		if(buyer_has>quantity){
			struct request maxi;
			showMax_max_heap(&buy_pq[item_no], &maxi);
			struct trade tr;
			tr.item_id = item_no;
			tr.seller_id = trader_id;
			tr.buyer_id = maxi.trader_id;
			tr.price = maxi.price;
			tr.quantity = quantity;

			insertIntoArray(tr, &trade_array[tr.seller_id]);
			insertIntoArray(tr, &trade_array[tr.buyer_id]);

			quantity -= tr.quantity;
			changeMaxQuantity_max_heap(&buy_pq[item_no], maxi.quantity - tr.quantity);
		}
		else{
			struct request maxi;
			extractMax(&buy_pq[item_no], &maxi);
			struct trade tr;
			tr.item_id = item_no;
			tr.seller_id = trader_id;
			tr.buyer_id = maxi.trader_id;
			tr.price = maxi.price;
			tr.quantity = maxi.quantity;

			insertIntoArray(tr, &trade_array[tr.seller_id]);
			insertIntoArray(tr, &trade_array[tr.buyer_id]);

			quantity -= tr.quantity;
		}
	}
	if(quantity){
		struct request temp;
		temp.trader_id = trader_id;
		temp.price = price;
		temp.quantity = quantity;
		insertIntoMinHeap(temp, &sell_pq[item_no]);
	}
	return;
}

void show_order_status(char message[]){
	for(int i=1;i<=10;i++){
		char header[50] = "Item No. ";
		char num[50];
		sprintf(num, "%d", i);
		char newl[50] = "\n";
		char bsheader[50] = "Best Sell = ";
		char quantityheader[50] = "\tQuantity = ";
		char bbheader[50] = "Best Buy = ";
		char bb[50] = "No Record", bs[50] = "No Record";
		char bbquant[50] = "No Record", bsquant[50] = "No Record";
		if(!isEmpty(&sell_pq[i])){
			int bsnum = min_price_min_heap(&sell_pq[i]);
			int bsquant_num = quantity_of_min_price_min_heap(&sell_pq[i]);
			sprintf(bs, "%d", bsnum);
			sprintf(bsquant, "%d", bsquant_num);
		}
		if(!isEmpty(&buy_pq[i])){
			int bbnum = max_price_max_heap(&buy_pq[i]);
			int bbquant_num = quantity_of_max_price_max_heap(&buy_pq[i]);
			sprintf(bb, "%d", bbnum);
			sprintf(bbquant, "%d", bbquant_num);
		}
		strcat(message, header);
		strcat(message, num);
		strcat(message, newl);
		strcat(message, bbheader);
		strcat(message, bb);
		strcat(message, quantityheader);
		strcat(message, bbquant);
		strcat(message, newl);
		strcat(message, bsheader);
		strcat(message, bs);
		strcat(message, quantityheader);
		strcat(message, bsquant);
		strcat(message, newl);
		strcat(message, newl);
	}
}

void show_trade_status(int trader_id, char message[]){
	int sz = getSize(&trade_array[trader_id]);
	for(int i=1;i<=sz;i++){
		struct trade current;
		getTradeByIndex(&trade_array[trader_id], &current, i);
		char iidheader[50] = "Item Id = ";
		char sidheader[50] = "Seller id = ";
		char bidheader[50] = "Buyer id = ";
		char pheader[50] = "Price = ";
		char qheader[50] = "Quantity = ";
		char iid[50], sid[50], bid[50], p[50], q[50];
		sprintf(iid, "%d", current.item_id);
		sprintf(sid, "%d", current.seller_id);
		sprintf(bid, "%d", current.buyer_id);
		sprintf(p, "%d", current.price);
		sprintf(q, "%d", current.quantity);
		char newl[50] = "\n";
		strcat(message, iidheader);
		strcat(message, iid);
		strcat(message, newl);
		strcat(message, sidheader);
		strcat(message, sid);
		strcat(message, newl);
		strcat(message, bidheader);
		strcat(message, bid);
		strcat(message, newl);
		strcat(message, pheader);
		strcat(message, p);
		strcat(message, newl);
		strcat(message, qheader);
		strcat(message, q);
		strcat(message, newl);
		strcat(message, newl);
	}
}

int main(int argc , char *argv[])   
{
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}
	int PORT = atoi(argv[1]);
	init();
	char *message;
    int opt = TRUE;   
    int master_socket, addrlen, new_socket, client_socket[5],  max_clients = 5, activity, i, valread, sd;   
    int max_sd;   
    struct sockaddr_in address;   
         
    char buffer[2049];  //data buffer of 2K  
         
    //set of socket descriptors  
    fd_set readfds;   
       
     
    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }   
         
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", PORT);   
         
    //try to specify maximum of 5 pending connections for the master socket  
    if (listen(master_socket, 5) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
         
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket,
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
           
            //send connection successful message
       		message = "Connection successful\nEnter username\n";
            
       		int usiz, psiz;

            write(new_socket, message, strlen(message));
                 
            char username[2049], password[2049];

            bzero(username,2049);
            usiz = read( new_socket , username, 2048);
            
            printf("Username = %s\n",username);

            message = "Enter password\n";
            write(new_socket , message , strlen(message));
            
            bzero(password,2049);
            psiz = read( new_socket , password, 2048);

            printf("Password = %s\n",password);
                 
            int ret = authenticate(username, password, usiz, psiz);

            if(ret==-3){
            	printf("Already logged in from somewhere else\nConnection closed\n");
            	message = "Already logged in from somewhere else\nConnection closed\n";
            	write(new_socket, message, strlen(message));

				getpeername(new_socket , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
        		printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  

        		close( new_socket );   
            }
            else if(ret==-2){
            	printf("Wrong password\nConnection closed\n");
            	message = "Wrong password\nConnection closed\n";
            	write(new_socket, message, strlen(message));

				getpeername(new_socket , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
        		printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  

        		close( new_socket );   
            }
            else if(ret==-1){
            	printf("No such user exists\nConnection closed\n");
            	message = "No such user exists\nConnection closed\n";
            	write(new_socket, message, strlen(message));

				getpeername(new_socket , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
        		printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  

        		close( new_socket );   
            }
            else{
            	printf("Authentication successful\n");
            	message = "Authentication successful\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
            	write(new_socket, message, strlen(message));
	            //add new socket to array of sockets  
	            for (i = 0; i < max_clients; i++)   
	            {   
	                //if position is empty  
	                if( client_socket[i] == 0 )   
	                {   
	                    client_socket[i] = new_socket;   
	                    logged_in[ret] = i;
	                    printf("Adding to list of sockets as %d\n" , i);   
	                         
	                    break;   
	                }   
	            }
            }   
        }   
        
        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++)   
        {   
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message
                bzero(buffer,2049);  
                if ((valread = read( sd , buffer, 2048)) == 0)   
                {   
                    //Somebody disconnected, get his details and print  
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );
                    for(int j=1;j<=5;j++){
                    	if(logged_in[j]==i){
                    		logged_in[j]=-1;
                    		break;
                    	}
                    }
                    client_socket[i] = 0;   
                }   
                
                else 
                {
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';
                    
                    if(valread==4&&(!(strncmp("buy\n" , buffer , 4)))){
                    	message="Enter item number (between 1 to 10 inclusive)\n";
                		send(sd , message , strlen(message) , 0 );
                		
                		bzero(buffer,2049);
					    read(sd,buffer,2048);
                		int itemno = atoi(buffer);
                		if(itemno>=1&&itemno<=10)
                		{
                			message="Enter quantity\n";
                			send(sd , message , strlen(message) , 0 );

                			bzero(buffer,2049);
					    	read(sd,buffer,2048);
                			int quant = atoi(buffer);
                			if(quant>0){
                				message="Enter unit price\n";
                    			send(sd , message , strlen(message) , 0 );

                    			bzero(buffer,2049);
						    	read(sd,buffer,2048);
                    			int price = atoi(buffer);
                    			if(price>0){
                    				message="Operation Successful\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                					send(sd , message , strlen(message) , 0 );
                    				int tid;
                    				for(int j=1;j<=5;j++){
                    					if(logged_in[j]==i){
                    						tid=j;
                    						break;
                    					}
                    				}
                    				buy(tid, itemno, quant, price);
                    			}
                    			else{
                    				message="Invalid price\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                					send(sd , message , strlen(message) , 0 );
                    			}
                			}	
                			else{
                				message="Invalid quantity\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                				send(sd , message , strlen(message) , 0 );
                			}
                		}
                		else{
                			message="Invalid item number\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                			send(sd , message , strlen(message) , 0 );
                		}
                    }
                    else if(valread==5&&(!(strncmp("sell\n" , buffer , 5)))){
                    	message="Enter item number (between 1 to 10 inclusive)\n";
                		send(sd , message , strlen(message) , 0 );
                		
                		bzero(buffer,2049);
					    read(sd,buffer,2048);
                		int itemno = atoi(buffer);
                		if(itemno>=1&&itemno<=10)
                		{
                			message="Enter quantity\n";
                			send(sd , message , strlen(message) , 0 );

                			bzero(buffer,2049);
					    	read(sd,buffer,2048);
                			int quant = atoi(buffer);
                			if(quant>0){
                				message="Enter unit price\n";
                    			send(sd , message , strlen(message) , 0 );

                    			bzero(buffer,2049);
						    	read(sd,buffer,2048);
                    			int price = atoi(buffer);
                    			if(price>0){
                    				message="Operation Successful\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                					send(sd , message , strlen(message) , 0 );
                    				int tid;
                    				for(int j=1;j<=5;j++){
                    					if(logged_in[j]==i){
                    						tid=j;
                    						break;
                    					}
                    				}
                    				sell(tid, itemno, quant, price);
                    			}
                    			else{
                    				message="Invalid price\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                					send(sd , message , strlen(message) , 0 );
                    			}
                			}	
                			else{
                				message="Invalid quantity\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                				send(sd , message , strlen(message) , 0 );
                			}
                		}
                		else{
                			message="Invalid item number\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                			send(sd , message , strlen(message) , 0 );
                		}
                    }
                    else if(valread==13&&(!(strncmp("order_status\n" , buffer , 13)))){
                    	bzero(buffer,2049);
                    	show_order_status(buffer);
                    	char success[100] = "Available commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                    	strcat(buffer, success);
                    	send(sd , buffer , strlen(buffer) , 0 );
                    }
                    else if(valread==13&&(!(strncmp("trade_status\n" , buffer , 13)))){
                    	int tid;
        				for(int j=1;j<=5;j++){
        					if(logged_in[j]==i){
        						tid=j;
        						break;
        					}
        				}
        				bzero(buffer,2049);
        				if(isEmpty_t(&trade_array[tid]))
        					strcpy(buffer, "No Record\n");
        				else
                    		show_trade_status(tid, buffer);
                    	char success[100] = "Available commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                    	strcat(buffer, success);

                    	send(sd , buffer , strlen(buffer) , 0 );

                    }
                    else if(valread==7&&(!(strncmp("logout\n" , buffer , 7)))){
                		message="Connection closed\n";
                		send(sd , message , strlen(message) , 0 );

						getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);   
                		printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  

                		close( sd );
                		for(int j=1;j<=5;j++){
	                    	if(logged_in[j]==i){
	                    		logged_in[j]=-1;
	                    		break;
	                    	}
	                    }   
                		client_socket[i] = 0;
                    }
                    else{
                    	message="Command not recognized\nAvailable commands:\n1. buy\n2. sell\n3. order_status\n4. trade_status\n5. logout\n";
                		send(sd , message , strlen(message) , 0 );
                    }
                }   
            }   
        }  
    }   
         
    return 0;   
} 
