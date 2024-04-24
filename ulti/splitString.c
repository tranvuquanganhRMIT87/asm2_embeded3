void splitString(char* src, char (*des)[100]){
	int i = 0;
	int j = 0;

	while (*src != '\0')
	{
		if(*src == ' '){
			i++;
			j = 0;
		}else{
			*(*(des+i) + j) = *src;
			j++;
		}
		src++;
	}
	
}