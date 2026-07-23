#if defined(__linux__)
    #include "/home/codeleaded/System/Static/Library/WindowEngine.h"
    #include "/home/codeleaded/System/Static/Library/ObjectNotation.h"
    #define C_SYNTAX "/home/codeleaded/System/SyntaxFiles/C_Syntax.alxon"
#elif defined(_WINE)
    #include "/home/codeleaded/System/Static/Library/WindowEngine.h"
    #include "/home/codeleaded/System/Static/Library/ObjectNotation.h"
    #define C_SYNTAX "/home/codeleaded/System/SyntaxFiles/C_Syntax.alxon"
#elif defined(_WIN32)
    #include "F:/home/codeleaded/System/Static/Library/WindowEngine.h"
    #include "F:/home/codeleaded/System/Static/Library/ObjectNotation.h"
    #define C_SYNTAX "F:/home/codeleaded/System/SyntaxFiles/C_Syntax.alxon"
#elif defined(__APPLE__)
    #error "Apple not supported!"
#else
    #error "Platform not supported!"
#endif


TextBox editor;
Button btn_save;
Button btn_load;

void Button_React_Save(void* parent,Button *b,ButtonEvent *be){
    if(be->eid == EVENT_PRESSED){
        Files_WriteT("./data/Output.c",editor.In.Buffer.Memory,editor.In.Buffer.size);
    }
}
void Button_React_Load(void* parent,Button *b,ButtonEvent *be){
    if(be->eid == EVENT_PRESSED){
        CStr cstr = Files_ReadT("./data/Output.c");
        String_Clear(&editor.In.Buffer);
        if(cstr){
            String_Append(&editor.In.Buffer,cstr);
            CStr_Free(&cstr);
        }
    }
}

void Setup(AlxWindow* w){
    editor = TextBox_New(Input_New(50,INPUT_MAXLENGTH),(Rect){ { 0.0f,0.0f },{ GetWidth(),GetHeight() } },ALXFONT_PATHS_HIGH,32,64,BLACK);
    TextBox_SetSyntax(&editor,C_SYNTAX);

    btn_save = Button_New(NULL,"Save",Button_React_Save,AlxFont_New(ALXFONT_HIGH),(Vec2){ 16,32 },(Rect){ { GetWidth() - 240.0f,10.0f },{ 110.0f,40.0f } },ALIGN_BORDER,0xFF00FF00,0xFFFFFFFF);
    btn_load = Button_New(NULL,"Load",Button_React_Load,AlxFont_New(ALXFONT_HIGH),(Vec2){ 16,32 },(Rect){ { GetWidth() - 120.0f,10.0f },{ 110.0f,40.0f } },ALIGN_BORDER,0xFF0000FF,0xFFFFFFFF);
}

void Update(AlxWindow* w){
    TextBox_Update(&editor,window.Strokes,GetMouse());
    Button_Update(&editor,&btn_save,window.Strokes,GetMouse(),GetMouseBefore());
    Button_Update(&editor,&btn_load,window.Strokes,GetMouse(),GetMouseBefore());

    Clear(BLACK);
    TextBox_Render(WINDOW_STD_ARGS,&editor);
    
    Button_Render(WINDOW_STD_ARGS,&editor,&btn_save);
    Button_Render(WINDOW_STD_ARGS,&editor,&btn_load);
}

void Delete(AlxWindow* w){
    Button_Free(&editor,&btn_load);
    Button_Free(&editor,&btn_save);
    TextBox_Free(&editor);
}

int main(){
    if(Create("Game Test",1900,1000,1,1,Setup,Update,Delete))
        Start();
    return 0;
}