int copyString(char *source, char* destination){
	
	while(*source != '\0'){
		*destination = *source;
		source++;
		destination++;
	}

	if (*source == '\0' && *destination == '\0'){
		return 0;
	}else {
		return -1;
	}
}