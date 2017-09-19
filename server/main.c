#include "head.h"

int main()
{
	char username[20]="xiaohong";
	char salt[20]={0};
	char sp[100]={0};
	sql_salt(username,salt);
	sql_sp(username,sp);
	printf("salt=%s,%ld\nsp=%s,%ld\n",salt,strlen(salt),sp,strlen(sp));
	return 0;
}
