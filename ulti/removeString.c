void removeString(char *string)
{
	int i = 0;
	while (string[i] != '\0')
	{
		string[i] = '\0';
		i++;
	}
}