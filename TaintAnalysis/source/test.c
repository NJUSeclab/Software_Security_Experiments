#include<stdio.h>
#include<string.h>
struct MyType{
	char input[10];
	int offset;
	int BUFF[100];
};
char * readstr(char *str){
	char c;
	int i=0;
	while((c=getchar()) !='\n'){
		str[i]=c;
		i++;
	}
	str[i]='\0';
	return str;
}

struct MyType Data;
int vulfun1(){
	Data.offset = 10;
	readstr(Data.input);
	*(Data.BUFF + Data.offset) = Data.input[0]+Data.input[1]+Data.input[3]+Data.input[4];
	return 0;
}
int vulfun2(){
	char buff[10];
	readstr(buff);
	return 0;
}
int main(){
	vulfun1();
	vulfun2();
	return 0;
}
