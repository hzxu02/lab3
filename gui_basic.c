#include <gtk/gtk.h>
#include <stdio.h>

// 버튼을 클릭했을 때 호출될 함수 (콜백 함수)
static void on_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("버튼이 클릭되었습니다! (Console Output)\n");
}

// 애플리케이션이 실행될 때 호출되는 함수
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box; // 레이아웃을 잡기 위한 박스

    // 1. 메인 윈도우 생성
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "실습 8: GUI 기초");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    // 2. 레이아웃 박스 생성 (수직 정렬, 간격 5)
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    // 3. 버튼 생성
    button = gtk_button_new_with_label("나를 눌러보세요");
    
    // 버튼 클릭 시그널("clicked")에 콜백 함수 연결
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    
    // 박스에 버튼 추가
    gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

    // 4. 모든 위젯을 화면에 표시
    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // GtkApplication 객체 생성
    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    
    // 'activate' 시그널에 함수 연결 (앱 시작 시 실행됨)
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    // 애플리케이션 실행 (메인 루프 진입)
    status = g_application_run(G_APPLICATION(app), argc, argv);
    
    // 메모리 해제
    g_object_unref(app);

    return status;
}