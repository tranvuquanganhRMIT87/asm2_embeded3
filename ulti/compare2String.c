int compare2String(char *input, char *command){

	while(*input == *command){
		if(*input == '\0' || *command == '\0'){
			break;
		}
		input++;
		command++;
	}

	if (*input == '\0' && *command == '\0'){
		return 0;
	}else {
		return -1;
	}
}