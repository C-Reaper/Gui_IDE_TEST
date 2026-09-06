#define C_SYNTAX "/home/codeleaded/System/SyntaxFiles/C_Syntax.json"
#include "/home/codeleaded/System/Static/Library/Scene.h"
#include "/home/codeleaded/System/Static/Library/LSP.h"
#include "/home/codeleaded/System/Static/Library/WindowEngine.h"

const char* clangd_args[] = {"clangd","--background-index",NULL};
const char* project = "/home/codeleaded/Hecke/C/Gui_IDE_Tiled_LSP";
const char* source = "/home/codeleaded/Hecke/C/Gui_IDE_Tiled_LSP/code/Code.c";
const char* uri = "file:///home/codeleaded/Hecke/C/Gui_IDE_Tiled_LSP/code/Code.c";
const char* root_uri = "file:///home/codeleaded/Hecke/C/Gui_IDE_Tiled_LSP";
LSP_Language clangd;
LSP lsp;
Scene scene;
CStr editor_content = NULL;
int version = 0;
static char* Read_File(const char* path){
    FILE* file = fopen(path,"rb");
    if(!file) return NULL;

    if(fseek(file,0,SEEK_END) != 0){ fclose(file); return NULL; }
    long size = ftell(file);
    if(size < 0){ fclose(file); return NULL; }
    rewind(file);

    char* data = malloc((size_t)size + 1U);
    if(!data){ fclose(file); return NULL; }

    size_t read = fread(data,1,(size_t)size,file);
    fclose(file);
    data[read] = '\0';
    return data;
}
static void Print_Message(const char* title,LSP_Package* package){
    LSP_MessageInfo info = {0};
    LSP_Message_Inspect(package,&info);

    printf("\n================ %s ================\n",title);

    if(info.type == LSP_MESSAGE_RESPONSE){
        printf("type: response\n");
        printf("id: %llu\n",(unsigned long long)info.id);
        printf("error: %s\n",info.error ? "yes" : "no");
    }else if(info.type == LSP_MESSAGE_NOTIFICATION){
        printf("type: notification\n");
        printf("method: %s\n",info.method ? info.method : "?");
    }else if(info.type == LSP_MESSAGE_REQUEST){
        printf("type: request\n");
        printf("id: %llu\n",(unsigned long long)info.id);
        printf("method: %s\n",info.method ? info.method : "?");
    }else{
        printf("type: invalid/unknown\n");
    }

    printf("%s\n",package->data ? package->data : "");
    printf("========================================\n");

    LSP_MessageInfo_Free(&info);
}
static char Wait_Print_Response(LSP* lsp,LSP_RequestID id,const char* title){
    LSP_Package package = LSP_Package_New();

    if(!LSP_Wait_Response(lsp,id,&package,5000U)){
        fprintf(stderr,"Timeout waiting for response %llu (%s).\n",
            (unsigned long long)id,title);
        return 0;
    }

    Print_Message(title,&package);
    LSP_Package_Free(&package);
    return 1;
}
static void Drain_Notifications(LSP* lsp,unsigned int milliseconds){
    uint64_t start = 0;
    struct timespec ts;
    timespec_get(&ts,TIME_UTC);
    start = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;

    while(1){
        LSP_Package package = LSP_Package_New();

        if(LSP_Poll(lsp,&package)){
            Print_Message("NOTIFICATION / SERVER MESSAGE",&package);
            LSP_Package_Free(&package);
            continue;
        }

        timespec_get(&ts,TIME_UTC);
        uint64_t now = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
        if(now - start >= milliseconds)
            break;

        Thread_Sleep_M(1);
    }
}

void Diagnostics_Update(Editor* editor){
	CStr content = String_CStr(&editor->In.Buffer);
	
	Files_Write((char*)source,content,CStr_Size(content));
    if(!LSP_DidChange(&lsp,uri,version++,content)){
        fprintf(stderr,"!!!!!!!! Could not send didChange.\n");
    }

	if(editor_content && !CStr_Cmp(editor_content,content)){
		Editor_Info_Clear(editor);
	}
	
	LSP_Package diagnostics = LSP_Package_New();
    while(LSP_Wait_Notification(&lsp,"textDocument/publishDiagnostics",&diagnostics,100U)){
        Print_Message("DIAGNOSTICS",&diagnostics);
		
		Json yl = Json_By(diagnostics.data);
		CStr name = Json_GetCStr(&yl,"params/diagnostics/0/code");
		CStr msg = Json_GetCStr(&yl,"params/diagnostics/0/message");
		
		Number start_line = Json_GetNumber(&yl,"params/diagnostics/0/range/start/line");
		Number start_char = Json_GetNumber(&yl,"params/diagnostics/0/range/start/character");
		Number end_line = Json_GetNumber(&yl,"params/diagnostics/0/range/end/line");
		Number end_char = Json_GetNumber(&yl,"params/diagnostics/0/range/end/character");
		
		if(name && msg){
			Editor_Info_Add(
				editor,
				Editor_Info_New(
					name,
					msg,
					start_line,
					start_char,
					end_line,
					end_char
				)
			);
		}
		//Json_Print(&yl);
    	//Json_Save(&yl,"./code/Save.json");
    	Json_Free(&yl);
		//char buffer[1024] = {0};
		//if(LSP_JSON_GetString(diagnostics.data,"params",(char**)&buffer)){
		//	printf("Found!!!!!!!!!!!!!!!!!!!\n");
		//}
        LSP_Package_Free(&diagnostics);
    }

	CStr_Free(&editor_content);
	editor_content = content;
}

void Component1_React(void* parent,TilingManager* b,TilingManagerEvent* be){

}
void Component2_React(void* parent,Button* b,ButtonEvent* be){
	
}
void Component3_React(void* parent,Editor* b,EditorEvent* be){
	if(be->eid == EVENT_PRESSED && be->ButtonId == ALX_MOUSE_L){
		Diagnostics_Update(b);
	}
}

void Setup(AlxWindow* w){
    clangd = LSP_Language_New("c","clangd",clangd_args);
    lsp = LSP_New();

    if(!LSP_Connect(&lsp,&clangd,project)){
        fprintf(stderr,"Failed to start clangd.\n");
        LSP_Free(&lsp);
        return;
    }

    LSP_RequestID initialize_id = LSP_Initialize(&lsp,root_uri);

    if(!initialize_id){
        fprintf(stderr,"Could not send initialize.\n");
        LSP_Free(&lsp);
        return;
    }
    if(!Wait_Print_Response(&lsp,initialize_id,"INITIALIZE")){
        LSP_Free(&lsp);
        return;
    }
    if(!LSP_Initialized(&lsp)){
        fprintf(stderr,"Could not send initialized notification.\n");
        LSP_Free(&lsp);
        return;
    }

   char* source_text = Read_File(source);
    if(!source_text){
        fprintf(stderr,"Could not read: %s\n",source);
        LSP_Shutdown(&lsp);
        LSP_Free(&lsp);
        return;
    }
    if(!LSP_DidOpen(&lsp,uri,"c",1,source_text)){
        fprintf(stderr,"Could not send didOpen.\n");
        free(source_text);
        LSP_Shutdown(&lsp);
        LSP_Free(&lsp);
        return;
    }
    free(source_text);

    /*
    const int line = 2;
    const int character = 4;

    LSP_RequestID hover_id = LSP_Hover(&lsp,uri,line,character);

    if(hover_id)
        Wait_Print_Response(&lsp,hover_id,"HOVER");

    LSP_RequestID completion_id = LSP_Completion(&lsp,uri,line,character);

    if(completion_id)
        Wait_Print_Response(&lsp,completion_id,"COMPLETION");

    LSP_RequestID definition_id = LSP_Definition(&lsp,uri,line,character);

    if(definition_id)
        Wait_Print_Response(&lsp,definition_id,"DEFINITION");

    LSP_RequestID declaration_id = LSP_Declaration(&lsp,uri,line,character);

    if(declaration_id)
        Wait_Print_Response(&lsp,declaration_id,"DECLARATION");

    LSP_RequestID references_id = LSP_References(&lsp,uri,line,character);

    if(references_id)
        Wait_Print_Response(&lsp,references_id,"REFERENCES");

    LSP_RequestID symbols_id = LSP_DocumentSymbols(&lsp,uri);

    if(symbols_id)
        Wait_Print_Response(&lsp,symbols_id,"DOCUMENT SYMBOLS");

    LSP_RequestID signature_id = LSP_SignatureHelp(&lsp,uri,line,character);

    if(signature_id)
        Wait_Print_Response(&lsp,signature_id,"SIGNATURE HELP");
	*/

	scene = Scene_New(
		NULL,
		Rect_New(
			(Vec2){ 0.0f,0.0f },
			(Vec2){ 1.0f,1.0f }
		),
		BLACK
	);
	
	/*
	Scene_Add(&scene,(TilingManager[]){
		TilingManager_Make(
			(void*)&scene,
			Component1_React,
			Rect_New((Vec2){ 0.1f,0.1f },(Vec2){ 0.8f,0.8f }),
			GRAY,
			GREEN,
			(void*[]){
				(Button[]){
					Button_NewStd(
						(void*)&scene,
						"Check LSP",
						Component2_React,
						(Vec2){ 32.0f,32.0f },
						Rect_New((Vec2){ 0.0f,0.0f },(Vec2){ 0.25f,0.33f }),
						DARK_CYAN,
						BLACK
					) 
				},
				NULL
			},
			(unsigned int[]){
				sizeof(Button),
				0UL
			}
		) 
	},sizeof(TilingManager));
	*/
	
	Scene_Add(&scene,(TilingManager[]){
		TilingManager_New(
			(void*)&scene,
			Component1_React,
			Rect_New((Vec2){ 0.1f,0.1f },(Vec2){ 0.8f,0.8f }),
			GRAY,
			GREEN
		) 
	},sizeof(TilingManager));
}
void Update(AlxWindow* w){

	if(w->Strokes[ALX_MOUSE_L].PRESSED && w->Strokes[ALX_KEY_CTRL].DOWN){
		TilingManager* const b = (TilingManager*)scene.childs.First->Memory;
		
		char* source_text = Read_File(source);

		Editor new_c = Editor_NewStd(
			&b->renderable,
			source_text,
			Component3_React,
			(Vec2){ 32.0f,32.0f },
			Rect_New(Vec2_Div(Vec2_Sub(GetMouse(),b->renderable.rect.p),b->renderable.rect.d),(Vec2){ 0.25f,0.25f }),
			100U,
			DARK_GRAY,
			LIGHT_GRAY
		);

		free(source_text);

		TilingManager_Insert(b,&new_c,sizeof(new_c));
	}

	Scene_Adapt(&scene,GetWidth(),GetHeight());
	Scene_Update(&scene);

	Scene_Input(&scene,window.Strokes,GetMouse(),GetMouseBefore());

	Clear(BLACK);

	Scene_Render(WINDOW_STD_ARGS,&scene);
}
void Delete(AlxWindow* w){
	CStr_Free(&editor_content);
	LSP_DidClose(&lsp,uri);

    LSP_RequestID shutdown_id = LSP_Shutdown(&lsp);

    if(shutdown_id)
        Wait_Print_Response(&lsp,shutdown_id,"SHUTDOWN");

    LSP_Exit(&lsp);
    LSP_Free(&lsp);

	Scene_Free(&scene);
}

int main(){
    if(Create("Tiled Editor with LSP",1900,1000,1,1,Setup,Update,Delete))
        Start();
    return 0;
}