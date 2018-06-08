#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
Something like Python

>> y = 2
>> z = 2
>> x = 3*y + 4/(2*z)

*/


/*
the only type: integer
everything is an expression
  statement   := END | expr END
  expr        := term expr_tail
  expr_tail   := ADDSUB term expr_tail | NIL
  term        := factor term_tail
  term_tail := MULDIV factor term_tail | NIL
  factor      := INT | ADDSUB INT | ADDSUB ID | ID ASSIGN expr | ID | LPAREN expr RPAREN
*/

#ifndef __LEX__
#define __LEX__
#define MAXLEN 256
typedef enum {UNKNOWN, END, INT, ID, ADDSUB, MULDIV, ASSIGN,
              LPAREN, RPAREN
             } TokenSet;
char lexeme[MAXLEN];

extern int match (TokenSet token);
extern void advance(void);
#endif // __LEX__

static TokenSet getToken(void);
static TokenSet lookahead = UNKNOWN;

int xlocation=0 ,ylocation=4 ,zlocation=8 ,notmul=1 ,notid=1 ,xid=0 ,yid=0 ,zid=0;
int mulsecond=0 ,first=1 ,xval=0 ,yval=0 ,zval=0 ,notinpatt=1 ,firstpatt=1 ,inpattfirst=1 ,ispatt=0 ,connectpatt=0 ,endpatt=0;
char judge=0;

void funnotmulinadd_exprhavefirst(int retval)
{
    if(notmul==1)
    {
        if(notid)
        {
            printf("MOV r2, %d\n",retval);

        }else
        {
            if(xid)
                printf("MOV r2, [0]\n");
            if(yid)
                printf("MOV r2, [4]\n");
            if(zid)
                printf("MOV r2, [8]\n");
            xid=0;
            yid=0;
            zid=0;

        }
        if(notinpatt)
            {printf("ADD r0, r2\n");
            first=0;}
        else
            {printf("ADD r3, r2\n");
        inpattfirst=0;}

    }
}

void funnotmulinadd_exprADD(int retval)
{
    if(notmul==1)
    {
        if(notid)
            printf("MOV r2, %d\n",retval);
        else
        {
            if(xid)
                printf("MOV r2, [0]\n");
            if(yid)
                printf("MOV r2, [4]\n");
            if(zid)
                printf("MOV r2, [8]\n");
            xid=0;
            yid=0;
            zid=0;

        }
        if(notinpatt)
            printf("ADD r0, r2\n");
        else
            printf("ADD r3, r2\n");
    }
    else
    {
        if(notinpatt)
            printf("ADD r0, r1\n");
        else
            printf("ADD r3, r1\n");
    }
}

void funnotmulinadd_exprSUB(int retval)
{
    if(notmul==1)
    {
        if(notid)
            printf("MOV r2, %d\n",retval);
        else
        {
            if(xid)
                printf("MOV r2, [0]");
            if(yid)
                printf("MOV r2, [4]");
            if(zid)
                printf("MOV r2, [8]");
            xid=0;
            yid=0;
            zid=0;

        }
        if(notinpatt)
            printf("SUB r0, r2\n");
        else
            printf("SUB r3, r2\n");
    }
    else
    {
        if(notinpatt)
            printf("SUB r0, r1\n");
        else
            printf("SUB r3, r1\n");
    }
}


void funnotidinterm(int num,int retval)
{
    if(notid)
        printf("MOV r%d, %d\n",num,retval);
    else
    {
        if(xid)
            printf("MOV r%d, [0]\n",num);
        if(yid)
            printf("MOV r%d, [4]\n",num);
        if(zid)
            printf("MOV r%d, [8]\n",num);
        xid=0;
        yid=0;
        zid=0;

    }
}

TokenSet getToken(void)
{
    int i;
    char c;

    while ( (c = fgetc(stdin)) == ' ' || c== '\t' );  // deal with space

    if (isdigit(c))
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i<MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    }
    else if (c == '+' || c == '-')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    }
    else if (c == '*' || c == '/')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    else if (c == '\n'||c == EOF)
    {
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=')
    {
        strcpy(lexeme, "=");
        return ASSIGN;
    }
    else if (c == '(')
    {
        strcpy(lexeme, "(");
        return LPAREN;
    }
    else if (c == ')')
    {
        strcpy(lexeme, ")");
        return RPAREN;
    }
    else if (isalpha(c) || c == '_')
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isalpha(c) || isdigit(c) || c == '_')
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    }
    else
    {
        printf("EXIT 1\n");
        exit(0);
    }
}

void advance(void)
{
    lookahead = getToken();
}

int match(TokenSet token)
{
    if (lookahead == UNKNOWN) advance();
    return token == lookahead;
}



#define TBLSIZE 65535
typedef struct
{
    char name[MAXLEN];
    int val;
} Symbol;
Symbol table[TBLSIZE];
int sbcount = 0;

int statement(void);
int expr(void);
int term(void);
int factor(void);
int getval(void);
int setval(char*, int);

typedef enum {MISPAREN, NOTNUMID, NOTFOUND, RUNOUT} ErrorType;
void error(ErrorType errorNum);


int getval(void)
{
    int i, retval, found;

    if (match(INT))
    {
        retval = atoi(lexeme);
    }
    else if (match(ID))
    {
        i = 0;
        found = 0;
        retval = 0;
        while (i<sbcount && !found)
        {
            if (strcmp(lexeme, table[i].name)==0)
            {
                retval = table[i].val;
                found = 1;
                break;
            }
            else
            {
                i++;
            }
        }
        if (!found)
        {
            if (sbcount < TBLSIZE)
            {
                strcpy(table[sbcount].name, lexeme);
                table[sbcount].val = 0;
                sbcount++;
            }
            else
            {
                error(RUNOUT);
            }
        }
    }
    return retval;
}

int setval(char *str, int val)
{
    int i, retval;
    i = 0;
    while (i<sbcount)
    {
        if (strcmp(str, table[i].name)==0)
        {
            table[i].val = val;
            retval = val;
            break;
        }
        else
        {
            i++;
        }
    }
    return retval;
}



int expr(void)
{
    int retval, retval1, retval2;

    retval = term();
    retval1 = retval;

    funnotmulinadd_exprhavefirst(retval1);

    while (match(ADDSUB))
    {
        if (strcmp(lexeme, "+")==0)
        {
            advance();
            retval = term();
            retval2 = retval;
            retval += retval;
        if(!endpatt)
            funnotmulinadd_exprADD(retval2);
        else
        {
            endpatt=0;




            if(notmul==1)
    {
        if(notid)
            {printf("MOV r2, r3\n");
            printf("MOV r3, 0\n");}
        else
        {
            if(xid)
                printf("MOV r2, [0]\n");
            if(yid)
                printf("MOV r2, [4]\n");
            if(zid)
                printf("MOV r2, [8]\n");
            xid=0;
            yid=0;
            zid=0;

        }
        if(notinpatt)
            printf("ADD r0, r2\n");
        else
            printf("ADD r3, r2\n");
    }
    else
    {
        if(notinpatt)
            printf("ADD r0, r1\n");
        else
            printf("ADD r3, r1\n");
    }



        }
        }
        else if (strcmp(lexeme, "-")==0)
        {
            advance();
            retval = term();
            retval2 = retval;
            retval += retval;
        if(!endpatt)
            funnotmulinadd_exprSUB(retval2);
        else
        {
            endpatt=0;

            if(notmul==1)
    {
        if(notid)
            {printf("MOV r2, r3\n");
            printf("MOV r3, 0\n");}
        else
        {
            if(xid)
                printf("MOV r2, [0]");
            if(yid)
                printf("MOV r2, [4]");
            if(zid)
                printf("MOV r2, [8]");
            xid=0;
            yid=0;
            zid=0;

        }
        if(notinpatt)
            printf("SUB r0, r2\n");
        else
            printf("SUB r3, r2\n");
    }
    else
    {
        if(notinpatt)
            printf("SUB r0, r1\n");
        else
            printf("SUB r3, r1\n");
    }



        }
        }
        else
        {
            // Nil
        }
    }


    return retval;
}

int term(void)
{
    int retval ,retvalcal;
    retval = factor();
    retvalcal = retval;

    notmul=1;

    while (match(MULDIV))
    {
        if (strcmp(lexeme, "*")==0)
        {
            advance();
            if(mulsecond==0)
            {
                if(ispatt==0)
                    funnotidinterm(1,retval);
                else
                {

                    printf("MOV r1, r3\n");
                    ispatt=0;
                }
            }
            if(match(LPAREN))
                connectpatt=1;
            retval = factor();
            retvalcal = retvalcal*retval;
            if(ispatt==0)
                funnotidinterm(2,retval);
            else
            {
                printf("MOV r2, r3\n");
                ispatt=0;
            }

            printf("MUL r1, r2\n");
            notmul=0;
            if(notinpatt)
            {
                if(match(ADDSUB)||match(END))
                    if(first==1)
                    {
                        printf("ADD r0, r1\n");
                        endpatt=0;
                        printf("MOV r3, 0\n");
                        first=0;
                    }
            }
            else
            {
                if(match(ADDSUB)||match(END))
                    if(inpattfirst==1)
                    {
                        printf("ADD r3, r1\n");
                        first=0;
                    }

            }

        }
        else if (strcmp(lexeme, "/")==0)
        {
            advance();
            if(mulsecond==0)
            {
                if(ispatt==0)
                    funnotidinterm(1,retval);
                else
                {
                    printf("MOV r1, r3\n");
                    ispatt=0;
                }
            }
                if(match(LPAREN))
                connectpatt=1;
            retval = factor();
            if(retval!=0)
                retvalcal = retvalcal/retval;
            if(retval==0)
            {
                printf("EXIT 1\n");
                exit(0);
            }
            if(ispatt==0)
                funnotidinterm(2,retval);
            else
            {
                printf("MOV r2, r3\n");
                ispatt=0;
            }

            printf("DIV r1, r2\n");
            notmul=0;
            if(notinpatt)
            {
                if(match(ADDSUB)||match(END))
                    if(first==1)
                    {
                        printf("ADD r0, r1\n");
                        endpatt=0;
                        printf("MOV r3, 0\n");
                        first=0;
                    }
            }
            else
            {
                if(match(ADDSUB)||match(END))
                    if(inpattfirst==1)
                    {
                        printf("ADD r3, r1\n");
                        first=0;
                    }

            }

        }
        else
        {
            // Nil
        }
        mulsecond=1;
    }
    mulsecond=0;
    return retvalcal;
}

int factor(void)
{
    int retval = 0;
    char tmpstr[MAXLEN];
    notid = 1;
    if (match(INT))
    {
        retval = getval();
        advance();
        if (match(ID))
        {
            printf("EXIT 1\n");
            exit(0);
        }
    }
    else if (match(ID))
    {
        if(strcmp("x", lexeme)==0)
        {
            xid=1;
            retval=xval;
        }

        else    if(strcmp("y", lexeme)==0)

        {
            yid=1;
            retval=yval;
        }


        else    if(strcmp("z", lexeme)==0)
        {
            zid=1;
            retval=zval;

        }
        else
            printf("EXIT 1\n");
        notid=0;
        advance();

        if (match(INT))
        {
            printf("EXIT 1\n");
            exit(0);
        }
        /*   if (match(ASSIGN))
           {
               advance();
               retval = expr();
               retval = setval(tmpstr, retval);
           } */
    }
    else if (match(ADDSUB))
    {
        strcpy(tmpstr, lexeme);
        advance();
        if (match(ID) || match(INT))
        {
            retval = getval();
            if (strcmp(tmpstr, "-")==0)
            {
                retval = -retval;
            }
            advance();
        }
        else
        {
            error(NOTNUMID);
        }
    }
    else if (match(LPAREN))
    {
        ispatt=1;
        advance();
        notinpatt=0;
        retval = expr();
        if (match(RPAREN))
        {
            endpatt=1;
            advance();
            if (match(END)||match(ADDSUB))
                if(firstpatt==1&&first==1&&!connectpatt)
                {
                    printf("ADD r0, r3\n");
                    firstpatt=0;
                }
            notinpatt=1;
        }
        else
        {
            error(MISPAREN);
        }
    }
    else
    {
        error(NOTNUMID);
    }
    return retval;
}

void error(ErrorType errorNum)
{
    switch (errorNum)
    {
    case MISPAREN:
        printf("EXIT 1\n");
        break;
    case NOTNUMID:
        printf("EXIT 1\n");
        break;
    case NOTFOUND:
        printf("EXIT 1\n");
        break;
    case RUNOUT:
        printf("EXIT 1\n");
    }
    exit(0);
}

int statement(void)
{
    int retval;

    if (match(END))
    {
        advance();
    }
    else
    {
        retval = expr();
        if (match(END))
        {
            lookahead = UNKNOWN;
        }
    }
    return retval;
}

int main(void)
{
  //  freopen("outout.txt","w",stdout);
  //  freopen("inin.txt","r",stdin);
    printf("MOV r0, 0\nMOV r1, 0\nMOV r2, 0\nMOV r3, 0\nMOV [0], r0\nMOV [4], r1\nMOV [8], r2\n");

    while ((judge = fgetc(stdin)) != EOF )
    {
        ungetc(judge, stdin);
        while ( (judge = fgetc(stdin)) == ' ' || judge== '\t'||judge=='\n' );

        if(judge=='x'||judge=='y'||judge=='z')
        {
            if(judge=='x')
            {
                while ( (judge = fgetc(stdin)) == ' ' || judge== '\t' );
                if(judge!='=')
                {
                    printf("EXIT 1\n");
                    return 0;
                }
                xval=statement();
                first=1;
                firstpatt=1;
                printf("MOV [%d], r0\nMOV r0, 0\nMOV r1, 0\nMOV r2, 0\nMOV r3, 0\n", xlocation );
            }
            else if(judge=='y')
            {
                while ( (judge = fgetc(stdin)) == ' ' || judge== '\t' );
                if(judge!='=')
                {
                    printf("EXIT 1\n");
                    return 0;
                }
                yval=statement();
                first=1;
                firstpatt=1;
                printf("MOV [%d], r0\nMOV r0, 0\nMOV r1, 0\nMOV r2, 0\nMOV r3, 0\n", ylocation );
            }
            else if(judge=='z')
            {
                while ( (judge = fgetc(stdin)) == ' ' || judge== '\t' );
                if(judge!='=')
                {
                    printf("EXIT 1\n");
                    return 0;
                }
                zval=statement();
                first=1;
                firstpatt=1;
                printf("MOV [%d], r0\nMOV r0, 0\nMOV r1, 0\nMOV r2, 0\nMOV r3, 0\n", zlocation );
            }
        }
        else if(judge==EOF)
        {
            break;
        }
        else
        {
            printf("EXIT 1\n");
            return 0;
        }

    }

    printf("MOV r0, 0\nMOV r1, 0\nMOV r2, 0\nMOV r3, 0\nMOV r0, [0]\nMOV r1, [4]\nMOV r2, [8]\nEXIT 0\n");



    return 0;
}

/*192.168.136.1*//*CSY103062316*/
