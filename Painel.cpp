//+------------------------------------------------------------------+
//|                                Copyright 2024, MetaQuotes Software Corp. |
//|                                             https://www.metaquotes.net/ |
//+------------------------------------------------------------------+
//|                        PAINEL DETECTOR DE TENDÊNCIA                |
//|                                                                  |
//| Este Expert Advisor (EA) cria um painel de análise de múltiplos  |
//| ativos diretamente no gráfico do MetaTrader 5. O painel exibe    |
//| uma variedade de indicadores e dados de mercado de forma         |
//| consolidada, com o objetivo de auxiliar o trader na rápida       |
//| identificação de tendências e oportunidades.                     |
//+------------------------------------------------------------------+
#property strict

//--- Definições de cores para a interface do painel
#define clrDarkBg           C'28,28,28'       // #1C1C1C - Cinza escuro para o fundo principal
#define clrGridLines        C'28,28,28'       // #1C1C1C - Cinza escuro para as linhas da grade
#define clrHeaderText       C'192,192,192'    // #C0C0C0 - Cinza claro para texto de cabeçalho
#define clrNormalText       C'255,255,255'    // #FFFFFF - Branco para texto padrão
#define clrBuyGreen         C'0,255,153'      // #00FF99 - Verde forte para indicar compra/alta
#define clrSellRed          C'255,77,77'      // #FF4D4D - Vermelho suave para indicar venda/baixa
#define clrActiveTab        C'150,100,255'    // #9694FF - Lilás para a aba que está selecionada
#define clrInactiveTab      C'30,30,30'       // Cinza mais escuro para abas não selecionadas
#define clrHighlightBg      C'40,40,40'       // Fundo para células de dados destacadas
#define clrNeutralText      C'150,150,150'    // Cinza para texto neutro ou em estado lateral
#define clrWarning          C'255,200,0'      // Laranja para avisos ou estados de atenção
#define clrHeaderSymbols    C'220,210,255'    // #DCD2FF - Lilás bem fraquinho para fundo do cabeçalho do painel (RGB: 220,210,255)

//--- Definições de layout e dimensionamento do painel
#define COL_WIDTH           100               // Largura padrão para colunas de dados dos ativos
#define LABEL_COL_WIDTH     120               // Largura para a primeira coluna, que contém os rótulos (Status, Score, etc.)
#define ROW_HEIGHT          25                // Altura padrão de cada linha da tabela
#define MARGIN              10                // Margem interna e externa do painel
#define FONT_SIZE           10                 // Tamanho da fonte para o conteúdo das células
#define HEADER_FONT_SIZE    10                // Tamanho da fonte para cabeçalhos e títulos de abas
#define HEADER_HEIGHT       30                // Altura da área do cabeçalho (que contém as abas)

//--- Parâmetros de Entrada (configuráveis pelo usuário na interface do MT5)
input string SymbolsToMonitor = "WINQ25,DOLU25,EURUSD";  // Lista de ativos para monitorar, separados por vírgula
input bool ShowStatus = true;         // Exibir/Ocultar a linha de Status
input bool ShowAcoes = true;          // Exibir/Ocultar a linha de Ações
input bool ShowScore = true;          // Exibir/Ocultar a linha de Score
input bool ShowPontuation = true;     // Exibir/Ocultar a linha de Pontuação
input bool ShowPressaoDOM = true;     // Exibir/Ocultar a linha de Pressão do DOM
input bool ShowDelta = true;          // Exibir/Ocultar a linha de Delta
input bool ShowLiquidity = true;      // Exibir/Ocultar a linha de Liquidez
input bool ShowSpread = true;         // Exibir/Ocultar a linha de Spread
input bool ShowMAs = true;            // Exibir/Ocultar o bloco de Médias Móveis
input bool ShowIndicators = true;     // Exibir/Ocultar o bloco de Indicadores Técnicos
input bool ShowResults = true;        // Exibir/Ocultar o bloco de Resultados

//--- Variáveis Globais
string symbolArray[];                 // Array que armazenará os nomes dos símbolos a serem monitorados
int totalSymbols = 0;                 // Número total de símbolos no array
bool panelMinimized = false;          // Controla o estado do painel (minimizado/maximizado)
bool initialHiddenMode = true;        // Modo de ocultação inicial para objetos (não utilizado ativamente, mas pode ser útil)
int activeTab = 1;                    // Controla qual aba está atualmente ativa (1 ou 2)
int panelWidth = 0;                   // Largura total do painel, calculada dinamicamente
int panelHeight = 0;                  // Altura total do painel, calculada dinamicamente

//+------------------------------------------------------------------+
//| Função de Inicialização do Expert Advisor (EA)                   |
//| É executada uma única vez quando o EA é anexado ao gráfico.      |
//+------------------------------------------------------------------+
int OnInit()
{
   Print("[0000] DEBUG OnInit iniciado");
   // 1. Processa a string de entrada com os símbolos e os armazena no array global
   ProcessSymbols();
   
   // 2. Calcula as dimensões do painel com base nos itens que serão exibidos
   CalculatePanelSize();
   
   Print("[0001] activeTab inicial: ", activeTab);
   
   // Define a aba 2 como a aba inicial a ser exibida
   activeTab = 2; 
   
   // 3. Cria todos os objetos gráficos que compõem o painel
   CreatePanel();
   
   Print("003 - DEBUG [OnInit] Painel criado. Aba ativa: ", activeTab);
   
   return(INIT_SUCCEEDED); // Retorna sucesso na inicialização
}

//+------------------------------------------------------------------+
//| Processa a string de símbolos fornecida pelo usuário.            |
//| Separa os símbolos por vírgula e os armazena no array global.    |
//+------------------------------------------------------------------+
void ProcessSymbols()
{
   // Divide a string `SymbolsToMonitor` usando a vírgula como delimitador
   StringSplit(SymbolsToMonitor, ',', symbolArray);
   totalSymbols = ArraySize(symbolArray);
   
   // Se nenhum símbolo for fornecido, usa o símbolo do gráfico atual como padrão
   if(totalSymbols == 0)
   {
     symbolArray[0] = Symbol();
     totalSymbols = 1;
   }
}

//+------------------------------------------------------------------+
//| Calcula a largura e altura totais do painel.                     |
//| As dimensões dependem do número de símbolos e das seções ativas. |
//+------------------------------------------------------------------+
void CalculatePanelSize()
{
   int rows = 1; // Começa com 1 para a linha de cabeçalho dos símbolos
   
   // Adiciona uma linha para cada seção que está habilitada nos parâmetros de entrada
   if(ShowStatus) rows++;
   if(ShowAcoes) rows++;
   if(ShowScore) rows++;
   if(ShowPontuation) rows++;
   if(ShowPressaoDOM) rows++;
   if(ShowDelta) rows++;
   if(ShowLiquidity) rows++;
   if(ShowSpread) rows++;
   if(ShowMAs) rows += 5;         // 5 linhas para as médias móveis
   if(ShowIndicators) rows += 11; // 11 linhas para os indicadores
   if(ShowResults) rows += 5;     // 5 linhas para os resultados
   
   // Calcula a largura: margem + coluna de rótulos + (largura da coluna * número de símbolos) + margem
   panelWidth = MARGIN + LABEL_COL_WIDTH + (COL_WIDTH * totalSymbols) + MARGIN;
   // Calcula a altura: altura do cabeçalho + (altura da linha * número de linhas) + margem
   panelHeight = HEADER_HEIGHT + (ROW_HEIGHT * rows) + MARGIN;
   
   // Garante uma altura mínima para o painel não ficar muito pequeno
   if (panelHeight < 150) panelHeight = 150; 
}

//+------------------------------------------------------------------+
//| Orquestra a criação de todos os elementos visuais do painel.     |
//+------------------------------------------------------------------+
void CreatePanel()
{
   Print("[0002] DEBUG Criando painel completo...");
   // Cria o retângulo de fundo principal do painel
   CreateRectLabel("TD_MainBg", MARGIN, MARGIN, panelWidth - (2*MARGIN), panelHeight - (2*MARGIN), clrDarkBg, clrGridLines, false);
   
   // Cria o cabeçalho, que inclui o título e os botões das abas
   CreateHeader();
   
   Print("[0003] Criando Tab 1...");
   // Cria todos os objetos da primeira aba (inicialmente ocultos)
   CreateTab1();
   
   Print("[0004] Criando Tab 2...");
   // Cria todos os objetos da segunda aba (inicialmente ocultos)
   //  CreateTab2();
   
   Print("[0005] Chamando SwitchTab com activeTab=", activeTab);
   // Controla a visibilidade para mostrar apenas a aba ativa
   SwitchTab(activeTab);
}

//+------------------------------------------------------------------+
//| Cria a seção do cabeçalho do painel.                             |
//| Inclui o fundo, os botões das abas, o título e o botão de       |
//| minimizar/maximizar.                                             |
//+------------------------------------------------------------------+
void CreateHeader()
{
   int x = MARGIN;
   int y = MARGIN;

   // Fundo do cabeçalho
   CreateRectLabel("TD_HeaderBg", x, y, panelWidth - (2*MARGIN), HEADER_HEIGHT, clrDarkBg, clrGridLines);
   
   // Botões para alternar entre as abas
   CreateTabButton("TD_Tab1", "Guia 1", x + 5, y + 5, 60, 20, activeTab == 1);
   CreateTabButton("TD_Tab2", "Guia 2", x + 70, y + 5, 60, 20, activeTab == 2);
   
   // Título do painel
   CreateLabel("TD_Title", "Detector de Tendência", x + 140, y + 5, clrHeaderText, HEADER_FONT_SIZE, "Arial", true);
   
   // Botão de minimizar/maximizar (o ícone muda dependendo do estado)
   CreateLabel("TD_MinimizeBtn", panelMinimized ? "[▲]" : "[▼]", x + panelWidth - (2*MARGIN) - 30, y + 5, clrHeaderText, HEADER_FONT_SIZE);
   ObjectSetInteger(0, "TD_MinimizeBtn", OBJPROP_SELECTABLE, true); // Torna o botão clicável
}

//+------------------------------------------------------------------+
//| Cria todo o conteúdo da primeira aba (a tabela de dados).        |
//| Cada linha é criada por uma função auxiliar.                     |
//+------------------------------------------------------------------+
void CreateTab1()
{
   int x = MARGIN;
   int y = MARGIN + HEADER_HEIGHT; // Posição inicial Y abaixo do cabeçalho
   int rowIndex = 0; // Contador para a posição vertical de cada linha
   
   // Cria o cabeçalho da tabela com os nomes dos símbolos
   CreateTableHeader(x, y + (rowIndex++ * ROW_HEIGHT));
   
   // Cria cada linha da tabela condicionalmente, com base nas configurações de entrada
   if(ShowStatus)
      CreateStatusRow(x, y + (rowIndex++ * ROW_HEIGHT));
   if(ShowAcoes)
      CreateAcoesRow(x, y + (rowIndex++ * ROW_HEIGHT));
   if(ShowScore)
      CreateScoreRow(x, y + (rowIndex++ * ROW_HEIGHT));
   if(ShowPontuation)
      CreatePontuationRow(x, y + (rowIndex++ * ROW_HEIGHT));
   if(ShowPressaoDOM)
      CreatePressaoDOMRow(x, y + (rowIndex++ * ROW_HEIGHT));
   if(ShowDelta)
      CreateDeltaRow(x, y + (rowIndex++ * ROW_HEIGHT));
   if(ShowLiquidity)
      CreateLiquidityRow(x, y + (rowIndex++ * ROW_HEIGHT));
   if(ShowSpread)
      CreateSpreadRow(x, y + (rowIndex++ * ROW_HEIGHT));
      
   // Bloco para criar as linhas das Médias Móveis
   if(ShowMAs)
   {
      CreateMARow(x, y + (rowIndex++ * ROW_HEIGHT), "MA");
      CreateMARow(x, y + (rowIndex++ * ROW_HEIGHT), "MA50");
      CreateMARow(x, y + (rowIndex++ * ROW_HEIGHT), "MA100");
      CreateMARow(x, y + (rowIndex++ * ROW_HEIGHT), "MA200");
      CreateMARow(x, y + (rowIndex++ * ROW_HEIGHT), "MA Higher TF");
   }
   
   // Bloco para criar as linhas dos Indicadores
   if(ShowIndicators)
   {
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "ADX");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "Ichimoku");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "PriceAction");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "MACD", true);
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "RSI");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "CCI");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "T3", true);
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "Ribbon", true);
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "BB");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "SAR");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "ATR");
      CreateIndicatorRow(x, y + (rowIndex++ * ROW_HEIGHT), "Volume");
   }
   
   // Bloco para criar as linhas de Resultados
   if(ShowResults)
   {
      CreateResultRow(x, y + (rowIndex++ * ROW_HEIGHT), "RR");
      CreateResultRow(x, y + (rowIndex++ * ROW_HEIGHT), "Auto", true);
      CreateResultRow(x, y + (rowIndex++ * ROW_HEIGHT), "Win/Loss", true);
      CreateResultRow(x, y + (rowIndex++ * ROW_HEIGHT), "Res. ativo", false, true);
      CreateResultRow(x, y + (rowIndex++ * ROW_HEIGHT), "Saldo Geral L/P:", false, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de cabeçalho da tabela com os nomes dos símbolos.   |
//+------------------------------------------------------------------+
void CreateTableHeader(int x, int y)
{
   // Cria uma célula vazia no canto superior esquerdo
   CreateCell("TD_Header_Label", x, y, " ", clrHeaderText, clrActiveTab, true, LABEL_COL_WIDTH, true);
   
   // Itera sobre todos os símbolos e cria uma célula de cabeçalho para cada um
   for(int i = 0; i < totalSymbols; i++)
   {
     CreateCell("TD_Header_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 symbolArray[i], clrHeaderText, clrHeaderSymbols, true, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Status" na tabela. (Função de exemplo)          |
//+------------------------------------------------------------------+
void CreateStatusRow(int x, int y)
{
   CreateCell("TD_Status_Label", x, y, "Status", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH, true);
   for(int i = 0; i < totalSymbols; i++)
   {
      string value = (i % 2 == 0) ? "✅" : "⚠️"; // Lógica de exemplo para alternar ícones
      CreateCell("TD_Status_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 value, clrNormalText, clrHighlightBg, false, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Ações" na tabela. (Função de exemplo)           |
//+------------------------------------------------------------------+
void CreateAcoesRow(int x, int y)
{
   CreateCell("TD_Acoes_Label", x, y, "Ações", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH, true);
   for(int i = 0; i < totalSymbols; i++)
   {
      string value = (i % 3 == 0) ? "▲ ▼ 🔴" : "▲ ▼ ⚪"; // Lógica de exemplo para alternar ícones
      CreateCell("TD_Acoes_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 value, clrNormalText, clrHighlightBg, false, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Score" na tabela. (Função de exemplo)           |
//+------------------------------------------------------------------+
void CreateScoreRow(int x, int y)
{
   CreateCell("TD_Score_Label", x, y, "SCORE", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH, true);
   for(int i = 0; i < totalSymbols; i++)
   {
      string value = (i % 2 == 0) ? "🟢 89" : "🔴 45"; // Lógica de exemplo
      color textColor = (i % 2 == 0) ? clrBuyGreen : clrSellRed;
      CreateCell("TD_Score_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 value, textColor, clrHighlightBg, false, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Pontuação" na tabela. (Função de exemplo)       |
//+------------------------------------------------------------------+
void CreatePontuationRow(int x, int y)
{
   CreateCell("TD_Pont_Label", x, y, "Pontuação", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH, true);
   for(int i = 0; i < totalSymbols; i++)
   {
      string value = (i % 3 == 0) ? "7-15-5" : "40-0-5"; // Lógica de exemplo
      CreateCell("TD_Pont_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 value, clrNormalText, clrHighlightBg, false, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Pressão DOM" na tabela. (Função de exemplo)     |
//+------------------------------------------------------------------+
void CreatePressaoDOMRow(int x, int y)
{
   CreateCell("TD_Pressao_Label", x, y, "PressaoDOM", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH, true);
   for(int i = 0; i < totalSymbols; i++)
   {
      int value = 80 + (i * 5); // Lógica de exemplo
      color textColor = (value > 70) ? clrBuyGreen : clrSellRed;
      CreateCell("TD_Pressao_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 IntegerToString(value), textColor, clrHighlightBg, false, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Delta" na tabela. (Função de exemplo)           |
//+------------------------------------------------------------------+
void CreateDeltaRow(int x, int y)
{
   CreateCell("TD_Delta_Label", x, y, "Delta", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH, true);
   for(int i = 0; i < totalSymbols; i++)
   {
      double value = (i % 2 == 0) ? 850.0 + (i * 10) : -720.0 - (i * 10); // Lógica de exemplo
      color textColor = (value >= 0) ? clrBuyGreen : clrSellRed;
      CreateCell("TD_Delta_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 DoubleToString(value, 0), textColor, clrHighlightBg, false, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Liquidez Maior" na tabela. (Função de exemplo)  |
//+------------------------------------------------------------------+
void CreateLiquidityRow(int x, int y)
{
   CreateCell("TD_Liquidez_Label", x, y, "Liquidez Maior", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH, true);
   for(int i = 0; i < totalSymbols; i++)
   {
      double value = 1.0825 + (i * 0.05); // Lógica de exemplo
      CreateCell("TD_Liquidez_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 DoubleToString(value, 4), clrNormalText, clrHighlightBg, false, COL_WIDTH, true);
   }
}

//+------------------------------------------------------------------+
//| Cria a linha de "Spread" na tabela. (Função de exemplo)          |
//+------------------------------------------------------------------+
void CreateSpreadRow(int x, int y)
{
   CreateCell("TD_Spread_Label", x, y, "Spread", clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH);
   for(int i = 0; i < totalSymbols; i++)
   {
      double value = 1.0 + (i * 0.5); // Lógica de exemplo
      CreateCell("TD_Spread_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 DoubleToString(value, 1), clrNormalText, clrHighlightBg, false, COL_WIDTH);
   }
}

//+------------------------------------------------------------------+
//| Cria uma linha genérica para uma Média Móvel (MA).               |
//+------------------------------------------------------------------+
void CreateMARow(int x, int y, string label)
{
   CreateCell("TD_MA_" + label + "_Label", x, y, label, clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH);
   for(int i = 0; i < totalSymbols; i++)
   {
      int value = 90 - (i * 5) + (label == "MA" ? 0 : 5); // Lógica de exemplo
      color textColor = (value > 80) ? clrBuyGreen : clrSellRed;
      CreateCell("TD_MA_" + label + "_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 IntegerToString(value), textColor, clrHighlightBg, false, COL_WIDTH);
   }
}

//+------------------------------------------------------------------+
//| Cria uma linha genérica para um Indicador Técnico.               |
//+------------------------------------------------------------------+
void CreateIndicatorRow(int x, int y, string label, bool isSpecialText = false)
{
   CreateCell("TD_Ind_" + label + "_Label", x, y, label, clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH);
   for(int i = 0; i < totalSymbols; i++)
   {
      string value;
      color textColor = clrNormalText;
      if (isSpecialText) // Para indicadores que retornam texto (ex: "Alta", "Baixa", "Lateral")
      {
         if (label == "MACD" || label == "T3" || label == "Ribbon")
         {
            value = (i % 2 == 0) ? "Lateral" : "Alta";
            textColor = (value == "Lateral") ? clrNeutralText : (value == "Alta" ? clrBuyGreen : clrSellRed);
         }
         else { value = "N/A"; }
      }
      else // Para indicadores que retornam um valor numérico
      {
         int numericValue = 90 - (i * 3); // Lógica de exemplo
         value = IntegerToString(numericValue);
         textColor = (numericValue > 70) ? clrBuyGreen : clrSellRed;
      }
      CreateCell("TD_Ind_" + label + "_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y, 
                 value, textColor, clrHighlightBg, false, COL_WIDTH);
   }
}

//+------------------------------------------------------------------+
//| Cria uma linha genérica para a seção de Resultados.              |
//+------------------------------------------------------------------+
void CreateResultRow(int x, int y, string label, bool isSpecialText = false, bool isCurrency = false)
{
   CreateCell("TD_Res_" + label + "_Label", x, y, label, clrHeaderText, clrDarkBg, true, LABEL_COL_WIDTH);
   for(int i = 0; i < totalSymbols; i++)
   {
      string value = "";
      color textColor = clrNormalText;

      if (isCurrency) // Formata o valor como moeda
      {
         if (label == "Saldo Geral L/P:") // Caso especial para o saldo geral, que ocupa uma célula
         {
            if (i == 0) {
               double amount = 250.00;
               value = "R$ " + StringFormat("%.2f", amount);
               textColor = clrBuyGreen;
            } else {
               value = " "; // Deixa as outras células da linha em branco
            }
         } else {
            double amount = (i % 2 == 0) ? -50.00 : 100.00;
            value = "R$ " + StringFormat("%.2f", amount);
            textColor = (amount >= 0) ? clrBuyGreen : clrSellRed;
         }
      }
      else if (isSpecialText) // Formata como texto especial (X ou placar)
      {
         if (label == "Auto") value = "X";
         else if (label == "Win/Loss") value = StringFormat("%d/%d", (i * 2 + 3), (i * 3 + 7));
      }
      else // Formata como número decimal
      {
         double numValue = 1.2 + (i * 0.3);
         value = DoubleToString(numValue, 1);
      }
      CreateCell("TD_Res_" + label + "_" + IntegerToString(i), x + LABEL_COL_WIDTH + (i * COL_WIDTH), y,
                 value, textColor, clrHighlightBg, false, COL_WIDTH);
   }
}

//+------------------------------------------------------------------+
//| Cria o conteúdo da segunda aba. (Atualmente, um placeholder)     |
//+------------------------------------------------------------------+
void CreateTab2()
{
   int x = MARGIN;
   int y = MARGIN + HEADER_HEIGHT;

   // Cria o fundo da aba 2 (inicialmente oculto)
   CreateRectLabel("TD_Tab2_Bg", x, y, panelWidth - (2*MARGIN), panelHeight - HEADER_HEIGHT - MARGIN - 5, clrDarkBg, clrGridLines, true);

   Print("[0010] CreateTab2 chamada. Criando elementos da aba 2 como ocultos");

   // Cria os textos da aba 2 (inicialmente ocultos)
   CreateLabel("TD_Tab2_Content", "Conteúdo da Guia 2", x + 10, y + 10, clrNormalText, FONT_SIZE, "Arial", false, true);
   CreateLabel("TD_Tab2_Content2", "Aqui você pode adicionar informações sobre saldo da conta, histórico de trades, etc.", x + 10, y + 30, clrNormalText, FONT_SIZE, "Arial", false, true);
}


//+------------------------------------------------------------------+
//| Funções Auxiliares para Criação de Objetos Gráficos              |
//+------------------------------------------------------------------+

/**
 * @brief Cria um rótulo retangular no gráfico.
 * @param name          Nome do objeto (deve ser único).
 * @param x             Posição X no gráfico.
 * @param y             Posição Y no gráfico.
 * @param width         Largura do retângulo.
 * @param height        Altura do retângulo.
 * @param bgColor       Cor de fundo.
 * @param borderColor   Cor da borda.
 * @param hidden        Se o objeto deve ser criado oculto (default: true).
 */
void CreateRectLabel(string name, int x, int y, int width, int height, color bgColor, color borderColor, bool hidden=true)
{
   ObjectCreate(0, name, OBJ_RECTANGLE_LABEL, 0, 0, 0);
   ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
   ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x);
   ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(0, name, OBJPROP_XSIZE, width);
   ObjectSetInteger(0, name, OBJPROP_YSIZE, height);
   ObjectSetInteger(0, name, OBJPROP_BGCOLOR, bgColor);
   ObjectSetInteger(0, name, OBJPROP_BORDER_COLOR, borderColor);
   ObjectSetInteger(0, name, OBJPROP_BORDER_TYPE, BORDER_FLAT);
   ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false);
   ObjectSetInteger(0, name, OBJPROP_ZORDER, 0); // ZORDER 0 para fundos
   ObjectSetInteger(0, name, OBJPROP_HIDDEN, hidden); // Controla a visibilidade
   Print("[0101] DEBUG CreateRectLabel: ", name, ", hidden=", hidden);
}

/**
 * @brief Cria um rótulo de texto no gráfico.
 * @param name       Nome do objeto (deve ser único).
 * @param text       O texto a ser exibido.
 * @param x          Posição X no gráfico.
 * @param y          Posição Y no gráfico.
 * @param clr        Cor do texto.
 * @param fontSize   Tamanho da fonte.
 * @param font       Nome da fonte.
 * @param bold       Se o texto deve estar em negrito.
 * @param hidden     Se o objeto deve ser criado oculto (default: true).
 */
void CreateLabel(string name, string text, int x, int y, color clr, int fontSize=8, string font="Arial", bool bold=false, bool hidden=true)
{
    ObjectCreate(0, name, OBJ_LABEL, 0, 0, 0); // Cria o objeto de texto (label)
    ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER); // Define o canto de referência
    ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x); // Define a distância X em pixels
    ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y); // Define a distância Y em pixels
    ObjectSetString(0, name, OBJPROP_TEXT, text); // Define o texto exibido
    ObjectSetInteger(0, name, OBJPROP_COLOR, clr); // Define a cor do texto
    ObjectSetInteger(0, name, OBJPROP_FONTSIZE, bold ? fontSize + 2 : fontSize); // Define o tamanho da fonte (maior se negrito)
    ObjectSetString(0, name, OBJPROP_FONT, font); // Define o tipo de fonte
    ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false); // Torna o objeto não selecionável
    ObjectSetInteger(0, name, OBJPROP_ZORDER, 1); // Define a ordem Z (sobre os fundos)
    ObjectSetInteger(0, name, OBJPROP_HIDDEN, hidden); // Controla se o objeto está oculto
    Print("[0100] DEBUG CreateLabel: ", name, ", hidden=", hidden); // Log de depuração
}

/**
 * @brief Cria uma "célula" da tabela, que é uma combinação de um retângulo de fundo e um texto.
 * @param name       Prefixo do nome para os objetos da célula.
 * @param x          Posição X no gráfico.
 * @param y          Posição Y no gráfico.
 * @param text       Texto da célula.
 * @param textColor  Cor do texto.
 * @param bgColor    Cor de fundo da célula.
 * @param bold       Se o texto deve estar em negrito.
 * @param cellWidth  Largura da célula.
 * @param hidden     Se a célula deve ser criada oculta.
 */
void CreateCell(string name, int x, int y, string text, color textColor, color bgColor, bool bold=false, int cellWidth = COL_WIDTH, bool hidden=true)
{
   // Cria o fundo da célula
   CreateRectLabel(name + "_Bg", x, y, cellWidth, ROW_HEIGHT, bgColor, clrGridLines, hidden);
   // Cria o texto da célula, centralizado verticalmente
   CreateLabel(name + "_Text", text, x + 5, y + (ROW_HEIGHT / 2) - (FONT_SIZE / 2), textColor, FONT_SIZE, "Arial", bold, hidden);
}

/**
 * @brief Cria um botão de aba, que é uma célula com capacidade de clique.
 * @param name    Prefixo do nome para os objetos do botão.
 * @param text    Texto do botão.
 * @param x       Posição X no gráfico.
 * @param y       Posição Y no gráfico.
 * @param width   Largura do botão.
 * @param height  Altura do botão.
 * @param active  Se o botão representa a aba ativa.
 */
void CreateTabButton(string name, string text, int x, int y, int width, int height, bool active)
{
   color bgColor = active ? clrActiveTab : clrInactiveTab; // Define a cor de fundo com base no estado ativo
   CreateRectLabel(name + "_Bg", x, y, width, height, bgColor, clrGridLines, false); // Botões de aba nunca são ocultos
   CreateLabel(name + "_Text", text, x + 10, y + (height / 2) - (HEADER_FONT_SIZE / 2), clrHeaderText, HEADER_FONT_SIZE, "Arial", true, false);
   ObjectSetInteger(0, name + "_Bg", OBJPROP_SELECTABLE, true); // Torna o fundo do botão clicável
}

//+------------------------------------------------------------------+
//| Funções de Controle da Interface                                 |
//+------------------------------------------------------------------+

/**
 * @brief Alterna a visibilidade dos elementos para mostrar a aba correta.
 *        Esta é a função central que gerencia o que é exibido no painel.
 * @param tab O número da aba a ser exibida (1 ou 2).
 */
void SwitchTab(int tab)
{
    Print("DEBUG, tab=", tab);
   Print("[0200] SwitchTab acionado. Tab = ", tab, ", Minimized = ", panelMinimized);
   
   // 1. Atualiza a cor de fundo dos botões das abas para refletir a seleção
   ObjectSetInteger(0, "TD_Tab1_Bg", OBJPROP_BGCOLOR, (tab == 1) ? clrActiveTab : clrInactiveTab);
    Print("[0210] TD_Tab1_Bg -> ", (tab == 1) ? "ATIVA" : "INATIVA");
   ObjectSetInteger(0, "TD_Tab2_Bg", OBJPROP_BGCOLOR, (tab == 2) ? clrActiveTab : clrInactiveTab);

   // 2. Determina a visibilidade de cada aba com base na aba selecionada e no estado minimizado
   bool showTab1 = (tab == 1) && !panelMinimized;
   bool showTab2 = (tab == 2) && !panelMinimized;

   // 3. Itera por todos os objetos do gráfico para mostrar/ocultar os elementos da Aba 1
   for(int i = 0; i < ObjectsTotal(0); i++)
   {
      string name = ObjectName(0, i);
   
      // Verifica se o nome do objeto pertence à Aba 1 (tabela de dados)
      bool isTab1 =
         StringFind(name, "TD_Header_", 0) == 0 ||
         StringFind(name, "TD_Status_", 0) == 0 ||
         StringFind(name, "TD_Acoes_", 0) == 0 ||
         StringFind(name, "TD_Score_", 0) == 0 ||
         StringFind(name, "TD_Pont_", 0) == 0 ||
         StringFind(name, "TD_Pressao_", 0) == 0 ||
         StringFind(name, "TD_Delta_", 0) == 0 ||
         StringFind(name, "TD_Liquidez_", 0) == 0 ||
         StringFind(name, "TD_Spread_", 0) == 0 ||
         StringFind(name, "TD_MA_", 0) == 0 ||
         StringFind(name, "TD_Ind_", 0) == 0 ||
         StringFind(name, "TD_Res_", 0) == 0;
   
      // Define a propriedade HIDDEN do objeto. Se showTab1 for true, HIDDEN é false (mostra o objeto).
      if(isTab1)
         ObjectSetInteger(0, name, OBJPROP_HIDDEN, !showTab1);
         
      Print("010 - DEBUG [SwitchTab] ", name, " => ", !showTab1 ? "OCULTO" : "VISÍVEL");
   }

   // 4. Controla a visibilidade dos elementos da Aba 2 diretamente
   ObjectSetInteger(0, "TD_Tab2_Bg", OBJPROP_HIDDEN, !showTab2);
   ObjectSetInteger(0, "TD_Tab2_Content", OBJPROP_HIDDEN, !showTab2);
   ObjectSetInteger(0, "TD_Tab2_Content2", OBJPROP_HIDDEN, !showTab2);

   Print("[0220] TD_Tab2_Bg -> ", showTab2 ? "VISÍVEL" : "OCULTO");
   Print("[0221] TD_Tab2_Content -> ", showTab2 ? "VISÍVEL" : "OCULTO");
   Print("[0222] TD_Tab2_Content2 -> ", showTab2 ? "VISÍVEL" : "OCULTO");

   // 5. Atualiza a variável global da aba ativa
   activeTab = tab;
}

//+------------------------------------------------------------------+
//| Minimiza ou restaura o painel.                                   |
//+------------------------------------------------------------------+
void ToggleMinimize()
{
   panelMinimized = !panelMinimized; // Inverte o estado booleano
   
   Print("[0300] ToggleMinimize acionado. Novo estado: ", panelMinimized);

   // Ajusta a altura do fundo principal para corresponder ao estado minimizado/maximizado
   ObjectSetInteger(0, "TD_MainBg", OBJPROP_YSIZE, panelMinimized ? HEADER_HEIGHT + MARGIN : panelHeight - (2*MARGIN));
   
   // Chama SwitchTab para ocultar/mostrar o conteúdo das abas conforme necessário
   SwitchTab(activeTab);
   
   // Atualiza o ícone do botão para refletir o novo estado
   ObjectSetString(0, "TD_MinimizeBtn", OBJPROP_TEXT, panelMinimized ? "[▲]" : "[▼]");
}

//+------------------------------------------------------------------+
//| Função de Eventos do Gráfico                                     |
//| Captura eventos como cliques em objetos.                         |
//+------------------------------------------------------------------+
void OnChartEvent(const int id, const long &lparam, const double &dparam, const string &sparam)
{
   // Verifica se o evento é um clique em um objeto
   if(id == CHARTEVENT_OBJECT_CLICK)
   {
      // Se o clique foi em um objeto da Aba 1
      if(StringFind(sparam, "TD_Tab1") >= 0)
      {
         Print("Clicou na Guia 1");
         SwitchTab(1);
      }
      // Se o clique foi em um objeto da Aba 2
      else if(StringFind(sparam, "TD_Tab2") >= 0)
      {
         Print("Clicou na Guia 2");
         SwitchTab(2);
      }
      // Se o clique foi no botão de minimizar
      else if(sparam == "TD_MinimizeBtn")
      {
         ToggleMinimize();
      }
   }
}

//+------------------------------------------------------------------+
//| Função de Tick do Expert                                         |
//| É executada a cada nova cotação (tick) do mercado.               |
//+------------------------------------------------------------------+
void OnTick()
{
   // Chama a função para atualizar os valores dinâmicos do painel
   UpdatePanelValues();
}

//+------------------------------------------------------------------+
//| Atualiza os valores exibidos no painel.                          |
//| Esta função deve conter a lógica de cálculo dos indicadores.     |
//+------------------------------------------------------------------+
void UpdatePanelValues()
{
   // Se o painel estiver minimizado, não há necessidade de atualizar os valores
   if (panelMinimized) return;

   // Itera sobre cada símbolo monitorado
   for(int i = 0; i < totalSymbols; i++)
   {
      string symbol = symbolArray[i];
      
      // Exemplo de atualização para a linha "Score"
      double score = CalculateScore(symbol); // Calcula o valor (atualmente com dados de exemplo)
      string scoreText = (score > 50) ? "🟢 " + IntegerToString((int)score) : "🔴 " + IntegerToString((int)score);
      color scoreColor = (score > 50) ? clrBuyGreen : clrSellRed;
      // Atualiza o texto e a cor do objeto de texto correspondente
      ObjectSetString(0, "TD_Score_" + IntegerToString(i) + "_Text", OBJPROP_TEXT, scoreText);
      ObjectSetInteger(0, "TD_Score_" + IntegerToString(i) + "_Text", OBJPROP_COLOR, scoreColor);
      
      // =================================================================================
      // EXERCÍCIO: Implementar a lógica de atualização para as outras métricas aqui.
      // Descomente e adapte o bloco abaixo como exemplo para o Delta.
      /*
      double deltaValue = CalculateDelta(symbol);
      string deltaText = DoubleToString(deltaValue, 0);
      color deltaColor = (deltaValue >= 0) ? clrBuyGreen : clrSellRed;
      ObjectSetString(0, "TD_Delta_" + IntegerToString(i) + "_Text", OBJPROP_TEXT, deltaText);
      ObjectSetInteger(0, "TD_Delta_" + IntegerToString(i) + "_Text", OBJPROP_COLOR, deltaColor);
      */
      // Repita o processo para Pontuação, Pressão DOM, Liquidez, Spread, MAs, Indicadores e Resultados.
      // =================================================================================
   }
}

//+------------------------------------------------------------------+
//| Funções de Cálculo (ATUALMENTE COM DADOS DE EXEMPLO)             |
//+------------------------------------------------------------------+

/**
 * @brief Calcula o "Score" para um determinado símbolo.
 * @param symbol O símbolo para o qual o score será calculado.
 * @return Um valor de score (atualmente aleatório para demonstração).
 * @note SUBSTITUA a lógica de exemplo pela sua lógica de cálculo real.
 */
double CalculateScore(string symbol)
{
   // Lógica de exemplo: retorna um número aleatório entre 0 e 99.
   MathSrand(GetTickCount()); // Inicializa o gerador de números aleatórios
   return MathRand() % 100;
}

/**
 * @brief Calcula o "Delta" para um determinado símbolo.
 * @param symbol O símbolo para o qual o delta será calculado.
 * @return Um valor de delta (atualmente aleatório para demonstração).
 * @note SUBSTITUA a lógica de exemplo pela sua lógica de cálculo real.
 */
double CalculateDelta(string symbol)
{
   // Lógica de exemplo: retorna um número aleatório entre -1000 e 1000.
   // Exemplo real: return SymbolInfoInteger(symbol, SYMBOL_ASK) - SymbolInfoInteger(symbol, SYMBOL_BID);
   MathSrand(GetTickCount() + 1); // Usa uma seed diferente para não repetir o valor do Score
   return (MathRand() % 2000) - 1000;
}

/**
 * @brief Calcula a "Liquidez" para um determinado símbolo.
 * @param symbol O símbolo para o qual a liquidez será calculada.
 * @return Um valor de liquidez (atualmente aleatório para demonstração).
 * @note SUBSTITUA a lógica de exemplo pela sua lógica de cálculo real.
 */
double CalculateLiquidity(string symbol)
{
   // Lógica de exemplo: retorna um número aleatório entre 1.0 e 1.1.
   // Exemplo real: return SymbolInfoDouble(symbol, SYMBOL_VOLUME_REAL);
   MathSrand(GetTickCount() + 2);
   return 1.0 + (MathRand() % 1000) / 10000.0;
}

/**
 * @brief Calcula o "Spread" para um determinado símbolo.
 * @param symbol O símbolo para o qual o spread será calculado.
 * @return Um valor de spread (atualmente aleatório para demonstração).
 * @note SUBSTITUA a lógica de exemplo pela sua lógica de cálculo real.
 */
double CalculateSpread(string symbol)
{
   // Lógica de exemplo: retorna um número aleatório entre 0.0 e 5.0.
   // Exemplo real: return (SymbolInfoInteger(symbol, SYMBOL_SPREAD) * SymbolInfoDouble(symbol, SYMBOL_POINT));
   MathSrand(GetTickCount() + 3);
   return (MathRand() % 50) / 10.0;
}

// Implemente aqui as funções de cálculo para os outros indicadores:
// Ex: double CalculateMA(string symbol, ENUM_TIMEFRAMES timeframe, int period, int shift) { ... }
// Ex: double CalculateADX(string symbol, ENUM_TIMEFRAMES timeframe, int period, int shift) { ... }

//+------------------------------------------------------------------+
//| Função de Desinicialização do Expert                             |
//| É executada quando o EA é removido do gráfico.                   |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
   // Remove todos os objetos gráficos criados por este EA para não poluir o gráfico.
   // O prefixo "TD_" (Trend Detector) garante que apenas os nossos objetos sejam removidos.
   ObjectsDeleteAll(0, "TD_");
}
