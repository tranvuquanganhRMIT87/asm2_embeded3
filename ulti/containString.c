int containString(char *input, char *command){

	while(*input != '\0'){
		if(*input != *command){
			break;
		}
		input++;
		command++;
	}

	if (*input == '\0'){
		return 1;
	}else {
		return -1;
	}
}