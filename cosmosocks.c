
int main()
{
    // Get users name and print it in reverse order
    char name[100];
    printf("Enter your name: ");
    // Flush stdout
    fflush(stdout);
    scanf("%s", name);
    printf("Your name in reverse order is: ");
    for (int i = strlen(name) - 1; i >= 0; i--)
    {
        printf("%c", name[i]);
    }
    return 0;
}