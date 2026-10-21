// fwMSX -- editor de textos: textos em portugues. Ver editor_i18n.h.
#include "editor_i18n.h"

#include <unordered_map>

namespace ed {

std::string Tr(const std::string &en) {
    static const std::unordered_map<std::string, std::string> kPt = {
        // --- menus ---
        {"File", "Arquivo"}, {"Block", "Bloco"}, {"Text-layout", "Texto"}, {"Search", "Busca"}, {"Print", "Imprimir"},
        {"View", "Exibir"}, {"Options", "Opcoes"}, {"F1=Help", "Ajuda"}, {"Build", "Montar"},
        {"Assemble / Convert", "Montar / Converter"}, {"BASIC line...", "Linha do BASIC..."}, {"Dignified setup...", "Config. Basic Dignified..."}, {"Run on MSX", "Executar no MSX"}, {"Next message", "Proxima mensagem"},
        {"Previous message", "Mensagem anterior"}, {"Messages...", "Mensagens..."}, {"On word", "Sobre a palavra"},
        // File
        {"New", "Novo"}, {"Open...", "Abrir..."}, {"Save", "Salvar"}, {"Save As...", "Salvar como..."}, {"Merge...", "Mesclar..."},
        {"2nd window...", "Dividir (2a janela)..."}, {"Close window", "Fechar janela"}, {"Load more", "Carregar mais"},
        {"Write to cursor", "Gravar ate o cursor"}, {"Top of file", "Topo do arquivo"}, {"DOS shell", "Shell"}, {"eXit", "Sair"},
        // Block
        {"Copy", "Copiar"}, {"Move", "Mover"}, {"Delete", "Apagar"}, {"Set marker", "Marcar"}, {"Remove markers", "Remover marcas"},
        {"Find marker", "Ir para a marca"}, {"Indent...", "Indentar..."}, {"Unindent...", "Desindentar..."}, {"Output to file...", "Gravar bloco em arquivo..."},
        // Text-layout
        {"Paragraph format", "Formatar paragrafo"}, {"Document format...", "Formatar documento..."}, {"Center line", "Centralizar linha"},
        {"Setup...", "Configurar..."}, {"Word wrap", "Quebra de linha automatica"}, {"Insert page breaks...", "Inserir quebras de pagina..."},
        {"Remove page breaks...", "Remover quebras de pagina..."}, {"Expand tabs...", "Expandir tabulacoes..."}, {"Put in Tabs...", "Converter espacos em tabulacoes..."},
        {"Auto indent", "Indentacao automatica"},
        // Search
        {"Find string...", "Localizar..."}, {"Find and Replace...", "Localizar e substituir..."}, {"Continue fwd", "Continuar adiante"},
        {"Continue rev", "Continuar para tras"}, {"Line number...", "Ir para a linha..."}, {"Matching bracket", "Parentese correspondente"},
        {"Window Differences", "Diferencas entre paineis"}, {"Text in Block", "Texto do bloco"}, {"Complete word", "Completar palavra"}, {"Block Marker", "Marca do bloco"},
        // Print
        {"Print Block", "Imprimir bloco"}, {"Eject page", "Ejetar pagina"},
        // View
        {"Information display", "Informacoes"}, {"Outline display", "Estrutura (outline)"}, {"Language...", "Linguagem..."},
        {"Language", "Linguagem"}, {"(auto)", "(automatica)"}, {"Syntax highlighting", "Destaque de sintaxe"}, {"Language:", "Linguagem:"},
        // Options
        {"Display...", "Tela..."}, {"Mouse...", "Mouse..."}, {"Editor...", "Editor..."}, {"Save configuration...", "Salvar configuracao..."},
        // Help
        {"General", "Geral"}, {"Keys", "Teclas"}, {"About", "Sobre"},
        // --- botoes e dialogos ---
        {"OK", "OK"}, {"Cancel", "Cancelar"}, {"Yes", "Sim"}, {"No", "Nao"}, {"All", "Todos"}, {"Skip", "Pular"},
        {"Don't Save", "Nao salvar"}, {"Next", "Proxima"}, {"Previous", "Anterior"}, {"Top", "Topo"}, {"Cursor", "Cursor"},
        {"Open file", "Abrir arquivo"}, {"Save file as", "Salvar arquivo como"}, {"Merge file", "Mesclar arquivo"},
        {"File name:", "Nome do arquivo:"}, {"Dirs:", "Pastas:"},
        {"Search for text", "Localizar texto"}, {"Search & Replace", "Localizar e substituir"}, {"Find:", "Localizar:"}, {"Replace:", "Substituir por:"},
        {"Upper/lower case must match", "Diferenciar maiusculas de minusculas"}, {"Search backwards", "Procurar para tras"},
        {"Replace this occurrence?", "Substituir esta ocorrencia?"},
        {"Find line", "Ir para a linha"}, {"Line number:", "Numero da linha:"}, {"BASIC line", "Linha do BASIC"}, {"Line or error:", "Linha ou erro:"}, {"Indent", "Indentar"}, {"Unindent", "Desindentar"},
        {"Amount:", "Quantidade:"}, {"Output to file", "Gravar bloco em arquivo"},
        {"Text-layout setup", "Configuracao do texto"}, {"Line Length:", "Tamanho da linha:"}, {"1st line margin:", "Margem da 1a linha:"},
        {"Left Margin:", "Margem esquerda:"}, {"Begin formatting at:", "Comecar a formatar:"}, {"Top of paragraph", "No inicio do paragrafo"},
        {"Print setup", "Configuracao da impressao"}, {"Top margin:", "Margem superior:"}, {"Left margin:", "Margem esquerda:"},
        {"Page length:", "Linhas por pagina:"}, {"Formfeed between pages", "Salto de pagina entre paginas"},
        {"Formfeed after printing:", "Salto de pagina ao terminar:"}, {"One page", "Uma pagina"}, {"Two pages", "Duas paginas"},
        {"Information display", "Informacoes"}, {"Characters in edit buffer:", "Caracteres no buffer:"}, {"Words in edit buffer:", "Palavras no buffer:"},
        {"Lines in edit buffer:", "Linhas no buffer:"}, {"Character at cursor:", "Caractere no cursor:"},
        {"Display setup", "Configuracao da tela"}, {"Color:", "Cores:"}, {"Display lines:", "Linhas na tela:"}, {"Don't change", "Nao mudar"},
        {"Display scrollbar", "Mostrar barra de rolagem"}, {"Blinking bracket", "Realcar parentese correspondente"},
        {"Zoom window effects", "Efeitos de zoom nas janelas"}, {"Large cursor", "Cursor grande"}, {"Tab spacing:", "Largura da tabulacao:"},
        {"Mouse setup", "Configuracao do mouse"}, {"Mouse button:", "Botao do mouse:"}, {"Use left button", "Usar o botao esquerdo"},
        {"Use right button", "Usar o botao direito"}, {"Mouse scrolling delay:", "Atraso da rolagem:"}, {"No delay", "Sem atraso"},
        {"Sync to clock", "Sincronizar com o relogio"}, {"Adj mouse cursor speed", "Ajustar a velocidade do cursor"},
        {"Editor setup", "Configuracao do editor"}, {"Insert / overstrike mode:", "Modo Insert / sobrescrever:"}, {"Start in insert", "Comecar em Insert"},
        {"Start in overstrike", "Comecar sobrescrevendo"}, {"Never overstrike", "Nunca sobrescrever"}, {"Tab key:", "Tecla Tab:"},
        {"Inserts a tab", "Insere uma tabulacao"}, {"Inserts spaces", "Insere espacos"}, {"Moves to tab stop", "Vai para a proxima parada"},
        {"Backup file character:", "Caractere do arquivo de backup:"}, {"Delete backup files", "Nao manter arquivos de backup"},
        {"Alt alone selects menu", "Alt sozinho abre o menu"},
        {"Auto-save every (minutes, 0 = off):", "Salvar automaticamente a cada (minutos, 0 = nao):"},
        {"Also overwrite files that have a name", "Gravar tambem nos arquivos que ja tem nome"},
        {"Line numbers", "Numeros de linha"}, {"Auto completion", "Completar sozinho"},
        {"Auto-saved", "Salvo automaticamente"},
        {"Save configuration", "Salvar configuracao"}, {"Update:", "Atualizar:"}, {"current config file", "o arquivo de configuracao atual"},
        {"config for this directory", "a configuracao desta pasta"},
        {"This file has been changed.", "Este arquivo foi alterado."}, {"Save it before exiting?", "Salvar antes de sair?"},
        {"Save it before starting a new file?", "Salvar antes de comecar um arquivo novo?"}, {"Save it before opening another?", "Salvar antes de abrir outro?"},
        {"Save it before closing?", "Salvar antes de fechar?"},
        {"The editor's configuration has been changed.", "A configuracao do editor foi alterada."}, {"Save this configuration?", "Salvar esta configuracao?"},
        {"Insert page breaks starting at page...", "Inserir quebras de pagina a partir de..."}, {"Remove page breaks starting at...", "Remover quebras de pagina a partir de..."},
        {"Expand tabs to spaces starting at...", "Expandir tabulacoes em espacos a partir de..."},
        {"Replace left-hand spaces with tabs starting at...", "Trocar espacos iniciais por tabulacoes a partir de..."},
        {"General help", "Ajuda geral"}, {"Help on keys", "Ajuda das teclas"}, {"File menu help", "Ajuda do menu Arquivo"}, {"Block menu help", "Ajuda do menu Bloco"},
        {"Text-layout menu help", "Ajuda do menu Texto"}, {"Search menu help", "Ajuda do menu Busca"}, {"Print menu help", "Ajuda do menu Imprimir"},
        {"View menu help", "Ajuda do menu Exibir"}, {"Option menu help", "Ajuda do menu Opcoes"},
        // --- mensagens da barra de estado ---
        {"Text not found", "Texto nao encontrado"}, {"Nothing to search for", "Nada para procurar"}, {"No matching bracket", "Sem parentese correspondente"},
        {"No block marked", "Nenhum bloco marcado"}, {"Cursor is inside the block", "O cursor esta dentro do bloco"}, {"Block start set", "Inicio do bloco marcado"},
        {"Block end set", "Fim do bloco marcado"}, {"Nothing to undo", "Nada para desfazer"}, {"Nothing to redo", "Nada para refazer"},
        {"Difference found", "Diferenca encontrada"}, {"No differences", "Sem diferencas"}, {"One file ends first", "Um dos arquivos termina antes"},
        {"Configuration saved", "Configuracao salva"}, {"There is only one file", "Ha apenas um arquivo"},
        {"Open a second window first (F2)", "Divida a area primeiro (F2)"}, {"Mark a block inside one line", "Marque um bloco dentro de uma linha"},
    };
    const auto it = kPt.find(en);
    if (it != kPt.end()) return it->second;
    // "Rotulo: valor" (as linhas do dialogo de informacoes): traduz so' o rotulo
    const size_t colon = en.find(':');
    if (colon != std::string::npos) {
        const auto head = kPt.find(en.substr(0, colon + 1));
        if (head != kPt.end()) return head->second + en.substr(colon + 1);
    }
    return en;
}

} // namespace ed
