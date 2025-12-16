/*  

--- Integrantes do grupo --- 

- Paola Campos da Silva
- Kaio Campos Tadeu
- João Pedro Huppes Arenales

*/ 

// -----------------------------------------------------------
// Bibliotecas

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

// -----------------------------------------------------------
// Constantes

#define SIZE 10
#define QT_CARTAS 4

// -----------------------------------------------------------
// Structs

struct player {
    char nome[100];
    int num_questoes;
    float premio;
};

typedef struct player player;

struct pergunta {
    char nivel;
    char descricao[200];
    char alt[4][30];
    char alt_correta;
};

typedef struct pergunta pergunta;

// -----------------------------------------------------------
// Protótipos das Funções

int menu();

void menu_pergunta(int *h, pergunta x);

void embaralhe_cartas(char *c[]);

int mostre_carta(int x, char *c[]);

player jogo(FILE *arq);

char ajuda(int *help, char alternativa, char alt_correta);

void ajuda_plateia(char alt_correta, int index_correta, char *letras, char *abc_xuxa[]);

void ajuda_universitarios(char alt_correta, int index_correta, char *letras, char *abc_xuxa[]);

void ajuda_cartas(char alt_correta, int index_correta, char *letras, char *abc_xuxa[]);

char verifique_alternativa_ajuda(char *categoria, char alternativa, char *texto);

char itere_ajuda(int *help, char alternativa, char num_ajuda[], int *repetidas, int num_pergunta, char alt_correta);

int pergunte(int nivel, int *help, int *repetidas, int qte_p_feitas, FILE *arq);

void edite_leaderboard(FILE *arq, player novo);

void mostre_leaderboard(FILE *arq);

// -----------------------------------------------------------
// Main Script

int main(){
    // Deve ser chamada para "semear" o gerador de números aleatórios
    srand(time(NULL));

    // Possibilita a utilização de acentos nos prints
    setlocale(LC_ALL, "Portuguese");

    int op;

    // Abertura dos arquivos
    FILE *p, *l;
    p = fopen("perguntas.dat", "rb");
    l = fopen("leaderboard.dat", "rb+"); // Tenta encontrar um save de leaderboard

    // Caso não exista um save do leaderboard criamos um
    if (l == NULL) {
        l = fopen("leaderboard.dat", "wb+");

        // Inicializando o leaderboard
        player j = {"\0", 0, 0.0};

        player jogadores[SIZE];
        for (int i = 0; i < SIZE; i++) {
            jogadores[i] = j;
        }

        fwrite(&jogadores, sizeof(player), SIZE, l);
    }

    // Verificação dos arquivos
    if (p == NULL || l == NULL) {
        perror("Erro ao abrir o arquivo!");
        exit(1);
    }

    int n;

    while (1){
        op = menu();

        player save; // Jogador

        // Seleção das opções do Menu
        switch (op){
            case 1: // Start
                save = jogo(p);

                edite_leaderboard(l, save);
                break;

            case 2: // Leaderboard
                mostre_leaderboard(l);
                break;

            case 3: // Exit

                // Fechamento dos arquivos
                fclose(l);
                fclose(p);

                return 0;

            default:
                break;
        }
        printf("\n");
    }
    
    // Fechamento dos arquivos
    fclose(l);
    fclose(p);

    return 0;
}

// -----------------------------------------------------------
// Declarações das Funções

int menu(){
    int select;
    
    printf("== ------------\nJOGO DO MILHAO!\nNovo Jogo(1)\nLeaderboard(2)\nSair(3)\n== ------------\n");

    // Seleção da opção desejada
    while (1) {
        scanf("%d%*c", &select);

        if (select == 1 || select == 2 || select == 3) {
            return select;
        }
    }
}

void menu_pergunta(int *h, pergunta x){
    char letras[] = {'a', 'b', 'c', 'd'};
    char num_ajuda[] = {'1', '2', '3', '4'};
    int alt;

    // Pergunta
    printf("\n%s\n", x.descricao);

    // Alternativas
    for (int i = 0; i < 4; i++) {
        printf("%c) %s\n", letras[i], x.alt[i]);
    }

    // Ajuda
    printf("\n--- Ajuda ---\n");
    printf("[1] Pular pergunta (%dx)\n", h[0]);
    printf("[2] Pedir ajuda a plateia (%dx)\n", h[1]);
    printf("[3] Pedir ajuda aos universitarios (%dx)\n", h[2]);
    printf("[4] Pedir ajuda as cartas (%dx)\n", h[3]);
    printf("[5] Parar\n");
    printf("~~~~~\n");

    // Resposta do usuário ou seleção da ajuda
    printf("\nEntre com a sua opção: ");

    return;
}

void embaralhe_cartas(char *c[]){
    int i, j;
    char *temp;

    // Função de embaralhar elementos | index aleatório como nova origem de elemento do index i.
    for (i = QT_CARTAS - 1; i > 0; i--) {
        j = rand() % (i + 1);
        temp = c[i];
        c[i] = c[j];
        c[j] = temp;
    }
    
    return;
}

int mostre_carta(int x, char *c[]){
    int b;

    if (strcmp(c[x], "Às de Espadas") == 0){
        printf("\nÀS DE ESPADAS!!! O que singifica que uma das alternativas errada será descartada....\n");
        b = 1;
    }
    else if (strcmp(c[x], "2 de Ouros") == 0){
        printf("\n2 DE OUROS! Felizmente, isso significa que duas alternativas erradas serão descartadas!\n");
        b = 2;
    }
    else if (strcmp(c[x], "3 de Paus") == 0){
        printf("\n3 DE PAUS?!?! O QUE! Mas essa é uma grande demonstração de sorte... \nBem, pra sua felicidade isso significa que todas as alternativas erradas serão descartadas... (chato o-o)\n");
        b = 3;
    }

    return b;
}

player jogo(FILE *arq){
    player save = {"\0", 0, 0.0};
    int veracidade, lv = 1, acertos = 0, p_feitas[20], qt_ajuda[] = {3, 3, 3, 3};
    float b_valor[] = {1000.0, 10000.0, 100000.0, 1000000.0};

    // Preenche o vetor de perguntas já feitas como -1 | tais valores serão gradativamente substituidos pelos números das questões já realizadas
    for (int i = 0; i < 20; i++) {
        p_feitas[i] = -1;
    }

    printf("\nBem-vindo(a) ao Show do Milhão!\nInsira seu nome: "); // Intro do jogo
    scanf("%99[^\n]%*c", save.nome); // É permitido uso de nomes de tamanho = 99 letras, e espaços são aceitos.

    while (1) {

        if (acertos == 5) { // Próximo bloco
            printf("\nParabéns! Vamos para o próximo bloco.\n"); 

            acertos = 0; // Contador de questões do bloco é reiniciado
            lv++; 
            if (lv != 4){
                save.premio = b_valor[lv - 1]; // Em qualquer bloco -- sem ser o último, o valor do prêmio é atualizado.
            }
        }

        else if (lv == 4) {  // Caso especial da pergunta final!
            printf("\nÉ a hora da verdade! Enfim, a pergunta final!\n"); // Anuncia a pergunta final!
            
            printf("\n== ------------\n== Pergunta %d\n== Voce possui um total acumulado de R$ %.2f\n", save.num_questoes + 1, save.premio);
            veracidade = pergunte(lv, qt_ajuda, p_feitas, save.num_questoes, arq);
        
            // Quando uma pergunta é pulada
            while (veracidade == 2) {
                printf("\n== ------------\n== Pergunta %d\n== Voce possui um total acumulado de R$ %.2f\n", save.num_questoes + 1, save.premio);
                veracidade = pergunte(lv, qt_ajuda, p_feitas, save.num_questoes, arq);
            }
            // Quando o jogador escolhe parar na ajuda
            if (veracidade == 3) {
                printf("\nQue pena! você estava tão perto de conseguir o GRANDE PRÊMIO!\n");
                break;
            }
            // Quando o jogador acerta a pergunta
            if (veracidade == 1) {
                save.premio = b_valor[3];
                save.num_questoes += 1;

                printf("\nMeus parabéns, você acertou! Seu conhecimento é sensacional!\n"); // Anuncia que acertou a pergunta final!
                break;
            }
            else{
                printf("\nBoohoo...! Você chegou tão perto. :[\n"); // anuncia que errou tadinho
                break;
            }
        }
        else { // Caso genérico de mecânica de perguntas
            printf("\n== ------------\n== Pergunta %d\n== Voce possui um total acumulado de R$ %.2f\n", save.num_questoes + 1, save.premio);
            veracidade = pergunte(lv, qt_ajuda, p_feitas, save.num_questoes, arq);

            // Quando uma pergunta é pulada
            while (veracidade == 2) {
                printf("\n== ------------\n== Pergunta %d\n== Voce possui um total acumulado de R$ %.2f\n", save.num_questoes + 1, save.premio);
                veracidade = pergunte(lv, qt_ajuda, p_feitas, save.num_questoes, arq);
            }
            // Quando o jogador escolhe parar na ajuda
            if (veracidade == 3) {
                printf("\nOk! Chegou ao seu limite? Eu entendo, não são todos que conseguem alcançar o GRANDE PRÊMIO!!\n");
                break;
            }
            if (veracidade == 1) {
                printf("\nCerta a resposta! Próxima questão: \n");
                
                save.premio += b_valor[lv - 1];
                acertos += 1;
                save.num_questoes += 1;
            }
            else {
                printf("\nFim de jogo! Mais sorte na próxima!\n"); // Anuncia que errou, logo fim de jogo!
                break;
            }
        }
    }
    return save; // Retornamos a 'ficha' do jogador, para então armazená-la no leaderboard após uma run do jogo.
}

char ajuda(int *help, char alternativa, char alt_correta){
    int i, index_correta;
    char letras[] = {'a', 'b', 'c', 'd'};
    char *abc_xuxa[] = {"Amor", "Baixinho", "Coração", "Docinho"};

    // Salvamos o index da alt_correta para ser usado futuramente.
    for (i = 0; i < 4; i++) { 
        if (letras[i] == alt_correta) {
            index_correta = i;
            break;
        }
    }

    switch (alternativa) {
        case '2': // Ajuda da plateia
            ajuda_plateia(alt_correta, index_correta, letras, abc_xuxa);
            help[1]--;
            break;

        case '3': // Ajuda dos universitários
            ajuda_universitarios(alt_correta, index_correta, letras, abc_xuxa);
            help[2]--;
            break;

        case '4': // Ajuda das cartas
            ajuda_cartas(alt_correta, index_correta, letras, abc_xuxa);
            help[3]--;
            break;

        default:
            break;
    }

    printf("\nEntre com a sua opção: ");
    scanf("%c%*c", &alternativa);

    // Verificação de que o usuário digitou uma resposta e não uma ajuda
    alternativa = verifique_alternativa_ajuda("abcd", alternativa, "Lamento! Mas só é possível utilizar essa ajuda uma vez por pergunta! D:");

    return alternativa;
}

void ajuda_plateia(char alt_correta, int index_correta, char *letras, char *abc_xuxa[]){
    int i, j, num, inicio = 5, fim = 6;
    int grupos[4] = {0, 0, 0, 0};
    char plateia[30];
    
    // Atribuí as respostas a cada membro do grupo de acordo com as suas respectivas porcentagens
    for (i = 0; i < 30; i++) {
        inicio = 5;
        fim = 6;

        num = (rand() % 10) + 1; // Intervalo [1,10] aleatório
        for (j = 0; j < 4; j++) { 
            if (num <= 4){
                plateia[i] = alt_correta;
                grupos[index_correta]++;
                break;
            }
            else if (j != index_correta && inicio <= num && num <= fim) {
                plateia[i] = letras[j];
                grupos[j]++;
                break;
            }
            else if (j != index_correta) {
                inicio = inicio + 2;
                fim = fim + 2;
            }
        }
    }

    printf("\n==--- PLATEIA ---==\n");
    for (i = 0; i < 4; i++) {
        printf("%d %s indicaram a alternativa %c de \"%s\".\n", grupos[i], grupos[i] == 1 ? "pessoa" : "pessoas", toupper(letras[i]), abc_xuxa[i]);
    }

    return;
}

void ajuda_universitarios(char alt_correta, int index_correta, char *letras, char *abc_xuxa[]){
    int i, j, index, num, inicial = 8;
    char alunos[3];
    char *nomes[] = {"Wendisvaldo", "Glauber", "Carlos"}; // Nomes irados

    // Atribuí as respostas a cada universitário de acordo com as suas respectivas porcentagens
    for (i = 0; i < 3; i++) {
        inicial = 8;

        num = (rand() % 10) + 1; //  Intervalo [1,10] aleatório.

        for (j = 0; j < 4; j++) {
            if (num <= 7) {
                alunos[i] = alt_correta;
                index = index_correta;
                break;
            }
            else if (j != index_correta && num == inicial) {
                alunos[i] = letras[j];
                index = j;
                break;
            }
            else if (j != index_correta) {
                inicial++; 
            }
        }

        printf("Universitário %s indicou a alternativa %c de \"%s\".\n", nomes[i], toupper(alunos[i]), abc_xuxa[index]);
    }

    return;
}

void ajuda_cartas(char alt_correta, int index_correta, char *letras, char *abc_xuxa[]){
    int i, j, num, bonus, descartar;
    char tiradas[3];
    char *cartas[QT_CARTAS] = {"Rei de Copas", "Às de Espadas", "2 de Ouros", "3 de Paus"};

    embaralhe_cartas(cartas); // Chamada da função de embaralhamento do vetor.

    printf("\nCartas embaralhadas, agora... faça sua escolha!! (1 até 4)\n");
    while(1) {
        scanf("%d%*c", &num);
        
        if (num == 1 || num == 2 || num == 3 || num == 4) {
            break;
        }

        printf("Opção inválida! Tente novamente...\n");
    }

    num--; // Reduzimos o num, para então usarmos como index.

    printf("\nVocê escolheu a carta de numero %d, agora o momento da verdade....ERA UM:\n", num);

    if (strcmp(cartas[num], "Rei de Copas") == 0) {
        printf("\nRei de Copas! Infelizmente, isso significa que nenhuma alternativa será descartada... ;p\n");
        return;
    }
    else {
        bonus = mostre_carta(num, cartas); // Chamada da função que apresenta a carta escolhida, ela retorna o valor de alternativas retiradas.
        printf("\n");
    }

    for (i = 0; i < bonus; i++) {
        tiradas[i] = 'p'; // Inicialização do vetor 
    }

    i = 0;

    while (i < bonus) { // Retiramos aleatoriamente alternativas incorretas.
        for (j = 0; j < 4; j++) {
            if (i >= bonus) {
                break;
            }
            if (j != index_correta && letras[j] != tiradas[i]) {
                descartar = rand() % 4;

                while (descartar == index_correta) {
                    descartar = rand() % 4; // Intervalo [0,3] aleatório;
                }
                
                printf("A alternativa %c de \"%s\" foi descartada!\n", toupper(letras[j]), abc_xuxa[j]);
                tiradas[i] = letras[j];
                i++;
            }
        }
    }

    return;
}

char verifique_alternativa_ajuda(char *categoria, char alternativa, char *texto) {
    
    // Ao receber uma pergunta o usuário tem duas opções de interação:
    
    // responder com uma letra, o que seria o equivalente a responder uma pergunta.
    if (strcmp(categoria, "abcd") == 0) {
        while (alternativa != 'a' && alternativa != 'b' && alternativa != 'c' && alternativa != 'd') {
            printf("\n%s\n", texto);
            
            printf("\nEntre com a sua opção: ");
            scanf("%c%*c", &alternativa);
        }
    }

    // ou responder com um número, o que seria o equivalente a pedir uma ajuda.
    else if (strcmp(categoria, "12345") == 0) {
        while (alternativa != '1' && alternativa != '2' && alternativa != '3' && alternativa != '4' && alternativa != '5') {
            printf("\n%s\n", texto);
            
            printf("\nEntre com a sua ajuda: ");
            scanf("%c%*c", &alternativa);
        }
    }

    return alternativa;
}

char itere_ajuda(int *help, char alternativa, char num_ajuda[], int *repetidas, int num_pergunta, char alt_correta) {
    int i = 0, j, h, cont;

    while (i < 4) {
        // Quando o jogador escolhe parar na ajuda
        if (alternativa == '5') {
            return '5';
        }
        else if (alternativa == num_ajuda[i]) {

            // Verificação de disponibilidade da ajuda selecionada + verificação de disponibilidade de outras ajudas.
            if (help[i] == 0) {
                cont = 0;
                for (j = 0; j < 4; j++) {
                    if (help[j] == 0) {
                        cont++;
                    }
                }

                // Caso todas as ajudas tenham sido usadas, alertamos o usuário.
                if (cont == 4) {
                    printf("\nTodas as suas ajudas foram esgotadas!! DDD:\n");
                    
                    printf("\nEntre com a sua opção: ");
                    scanf("%c%*c", &alternativa);

                    // Verificação de que o usuário digitou uma resposta e não uma ajuda
                    alternativa = verifique_alternativa_ajuda("abcd", alternativa, "Lamento! Mas você realmente não possui mais ajuda! D:");
                    
                    break;
                }

                printf("\nEssa ajuda está esgotada! ;(\n");

                printf("\nDeseja ainda receber alguma ajuda? Sim(1) | Não(0)\n");
                
                while(1) {
                    scanf("%d%*c", &h);
                    if (h == 1 || h == 0) {
                        break;
                    }
                    printf("Opção inválida! Tente novamente...\n");
                }
                
                // Caso o usuário tenha escolhido receber ajuda
                if (h) {
                    printf("\nEntre com a ajuda desejada: ");
                    scanf("%c%*c", &alternativa);

                    // Verificação de que o usuário digitou uma ajuda e não uma resposta
                    alternativa = verifique_alternativa_ajuda("12345", alternativa, "Lamento! Mas você escolheu receber ajuda! D:");
                }
                else {
                    printf("\nEntre com a sua opção: ");
                    scanf("%c%*c", &alternativa);

                    // Verificação de que o usuário digitou uma resposta e não uma ajuda
                    alternativa = verifique_alternativa_ajuda("abcd", alternativa, "Lamento! Mas você escolheu que não desejaria ajuda! D:");

                    break;
                }
            }
            else if (alternativa == '1') { // Quando uma pergunta é pulada
                help[i]--;

                // Adiciona a pergunta repetida ao seu local destinado no vetor repetidas, do index 16 ao 19
                for (j = 16; j < 20; j++) {
                    if (repetidas[j] == -1) {
                        repetidas[j] = num_pergunta;
                        break;
                    }
                }

                return '1';
            }
            else if (alternativa == '2' || alternativa == '3' || alternativa == '4') {
                alternativa = ajuda(help, alternativa, alt_correta);
                break;
            }
        }
        // Incremento de sentinela | index
        if (h) {
            i = 0;
            h = 0;
        }
        else {
            i++;
        }
    }

    return alternativa;
}

int pergunte(int nivel, int *help, int *repetidas, int qte_p_feitas, FILE *arq){
    pergunta pergunta_atual;
    char alternativa;
    int veracidade = 0, inicio, fim, num_pergunta, i, j, inicio_repetidas;
    char num_ajuda[] = {'1', '2', '3', '4'};

    // Selecionando o nível das questões
    inicio = (nivel - 1) * 20;
    inicio_repetidas = (nivel - 1) * 5;

    // Curiosidade: caso houvesse um mesmo número de perguntas para todos os blocos podiamos somente ter feito fim = inicio + 19
    if (nivel == 4) {
        fim = inicio + 9;
    }
    else {
        fim = inicio + 19;
    }

    // Sortear uma pergunta
    num_pergunta = rand() % (fim - inicio + 1) + inicio; // Intervalo [inicio, fim] aleatório

    // Conferir se a pergunta atual já foi sorteada e utilizada anteriormente
    i = inicio_repetidas;
    while (i < qte_p_feitas) {
        if (num_pergunta == repetidas[16] || num_pergunta == repetidas[17] || num_pergunta == repetidas[18] || num_pergunta == repetidas[19]) {
            num_pergunta = rand() % (fim - inicio + 1) + inicio;
            i = inicio_repetidas;
        }
        else if (num_pergunta == repetidas[i]) {
            num_pergunta = rand() % (fim - inicio + 1) + inicio;
            i = inicio_repetidas;
        }
        else {
            i++;
        }
    }

    fseek(arq, num_pergunta * (long)sizeof(pergunta), SEEK_SET);
    fread(&pergunta_atual, sizeof(pergunta), 1, arq);

    /* somente para teste -- debbuger mode
    
    printf("\nPergunta atual: %d\n", num_pergunta);
    printf("Alternativa correta: %c\n", pergunta_atual.alt_correta);
    
    end debugger mode */
    
    menu_pergunta(help, pergunta_atual);
    scanf("%c%*c", &alternativa);

    if (alternativa != 'a' && alternativa != 'b' && alternativa != 'c' && alternativa != 'd') {
        // Verificação de que o usuário digitou uma ajuda
        alternativa = verifique_alternativa_ajuda("12345", alternativa, "Opção inválida! Insira uma ajuda válida...");
        
        alternativa = itere_ajuda(help, alternativa, num_ajuda, repetidas, num_pergunta, pergunta_atual.alt_correta);
        
        if (alternativa == '1') { // Quando pula a pergunta
            return 2;
        }
        else if (alternativa == '5') { // Quando escolhe parar
            return 3;
        }
    }

    // Verificação de que o usuário digitou uma resposta
    alternativa = verifique_alternativa_ajuda("abcd", alternativa, "Opção inválida! Insira uma alternativa válida...");

    // Adiciona a pergunta à lista de perguntas já feitas anteriormente
    if (qte_p_feitas < 16) {
        repetidas[qte_p_feitas] = num_pergunta;
    }

    // Verifica se a resposta do usuário está certa ou errada
    if (alternativa == pergunta_atual.alt_correta) {
        veracidade = 1;
    }

    return veracidade;
}

void edite_leaderboard(FILE *arq, player novo){
    
    // Cria-se um vetor de tamanho SIZE + 1, pois retornaremos somente os SIZE melhores, deixando o último (o pior) de fora do ranking.
    // Caso a pontuação seja igual, o que jogou o jogo primeiro recebe prioridade no ranking.

    player arq_to_vect[SIZE + 1];

    rewind(arq);

    size_t num_lidos = fread(arq_to_vect, sizeof(player), SIZE, arq); // Extraímos o ranking para um vetor.
    player temp;
    int i, sentinela = 1, pos_inserir = -1;

    // Caso o novo save tenha superado algum antigo, o inserimos em seu lugar
    for (i = 0; i < num_lidos; i++) { 
        if (novo.num_questoes > arq_to_vect[i].num_questoes) {
            pos_inserir = i;
            break;
        }
    }

    if (pos_inserir == -1) {
        if (num_lidos < SIZE) {
            pos_inserir = (int)num_lidos;
        }
        else {
            return;
        }
    }

    // Após isso, 'empurramos' para 'direita' os valores após a inserir (caso tenha sido inserido um novo recorde).
    // O último colocado do antigo ranking é então ignorado (pois somente gravamos no arquivo os 10 primerios do vetor).

    for (i = SIZE - 1; i >= pos_inserir; i--) {
        arq_to_vect[i + 1] = arq_to_vect[i];
    }

    arq_to_vect[pos_inserir] = novo;

    rewind(arq); 

    fwrite(arq_to_vect, sizeof(player), SIZE, arq); // Escrevemos no arquivo o vetor.
    fseek(arq, (long)sizeof(player) * SIZE, SEEK_SET); 
    
    return;
}

void mostre_leaderboard(FILE *arq){

    player arq_to_vect[SIZE];

    rewind(arq);
    fread(arq_to_vect, sizeof(player), SIZE, arq); // Copiamos o ranking para um vetor

    printf("== ------------\nNOME   QUESTÕES  PRÊMIO\n"); // Título
    for (int i = 0; i < SIZE; i++){
        printf("%-*.*s   %.2d  %.2f\n", SIZE, SIZE, arq_to_vect[i].nome, arq_to_vect[i].num_questoes, arq_to_vect[i].premio);
    }
    printf("== ------------\n");

    return;
}
