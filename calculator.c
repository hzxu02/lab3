#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 전역 변수: 계산 상태 저장
static GtkWidget *entry_display; // 결과 표시창
static double num1 = 0;          // 첫 번째 숫자
static int operation = 0;        // 연산자 (1:+, 2:-, 3:*, 4:/)
static int new_entry = 1;        // 새로운 숫자 입력 시작 여부 플래그

// 숫자 버튼 클릭 핸들러
static void on_num_clicked(GtkWidget *widget, gpointer data) {
    const char *num_str = (const char *)data;
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(entry_display));
    char new_text[100];

    // [로그] 입력된 숫자 출력
    g_print("[입력] 숫자 버튼 클릭: %s\n", num_str);

    if (new_entry) {
        gtk_entry_set_text(GTK_ENTRY(entry_display), num_str);
        new_entry = 0; 
    } else {
        sprintf(new_text, "%s%s", current_text, num_str);
        gtk_entry_set_text(GTK_ENTRY(entry_display), new_text);
    }
}

// 연산자 버튼 (+, -, *, /) 클릭 핸들러
static void on_op_clicked(GtkWidget *widget, gpointer data) {
    const char *op_str = (const char *)data;
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(entry_display));
    
    num1 = atof(current_text);
    new_entry = 1; 

    if (strcmp(op_str, "+") == 0) operation = 1;
    else if (strcmp(op_str, "-") == 0) operation = 2;
    else if (strcmp(op_str, "*") == 0) operation = 3;
    else if (strcmp(op_str, "/") == 0) operation = 4;

    // [로그] 연산자 선택 및 저장된 숫자 출력
    g_print("[연산] 연산자 선택: '%s' (첫 번째 숫자 저장: %.2f)\n", op_str, num1);
}

// 계산(=) 버튼 클릭 핸들러
static void on_calc_clicked(GtkWidget *widget, gpointer data) {
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(entry_display));
    double num2 = atof(current_text); // 두 번째 숫자
    double result = 0;
    char result_str[100];
    char op_char = '?';

    switch (operation) {
        case 1: result = num1 + num2; op_char = '+'; break;
        case 2: result = num1 - num2; op_char = '-'; break;
        case 3: result = num1 * num2; op_char = '*'; break;
        case 4: 
            if (num2 == 0) {
                gtk_entry_set_text(GTK_ENTRY(entry_display), "Error");
                g_print("[오류] 0으로 나눌 수 없습니다.\n");
                new_entry = 1;
                return;
            }
            result = num1 / num2; 
            op_char = '/';
            break;
        default: return;
    }

    // [로그] 상세 계산 과정 출력
    g_print("========================================\n");
    g_print("[결과] 계산 수행: %.2f %c %.2f = %.2f\n", num1, op_char, num2, result);
    g_print("========================================\n");

    sprintf(result_str, "%.2f", result);
    gtk_entry_set_text(GTK_ENTRY(entry_display), result_str);
    new_entry = 1; 
}

// 초기화(C) 버튼 클릭 핸들러
static void on_clear_clicked(GtkWidget *widget, gpointer data) {
    gtk_entry_set_text(GTK_ENTRY(entry_display), "0");
    num1 = 0;
    operation = 0;
    new_entry = 1;
    // [로그] 초기화 알림
    g_print("[초기화] 모든 내용이 지워졌습니다.\n");
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;
    
    const char *buttons[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "C", "0", "=", "+"
    };

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "계산기");
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 300);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_add(GTK_CONTAINER(window), grid);

    entry_display = gtk_entry_new();
    gtk_entry_set_alignment(GTK_ENTRY(entry_display), 1); 
    gtk_editable_set_editable(GTK_EDITABLE(entry_display), FALSE); 
    gtk_entry_set_text(GTK_ENTRY(entry_display), "0");
    
    gtk_grid_attach(GTK_GRID(grid), entry_display, 0, 0, 4, 1);

    for (int i = 0; i < 16; i++) {
        button = gtk_button_new_with_label(buttons[i]);
        
        int x = i % 4;     
        int y = (i / 4) + 1; 

        if (strcmp(buttons[i], "=") == 0) {
            g_signal_connect(button, "clicked", G_CALLBACK(on_calc_clicked), NULL);
        } else if (strcmp(buttons[i], "C") == 0) {
            g_signal_connect(button, "clicked", G_CALLBACK(on_clear_clicked), NULL);
        } else if (strchr("+-*/", buttons[i][0])) { 
            g_signal_connect(button, "clicked", G_CALLBACK(on_op_clicked), (gpointer)buttons[i]);
        } else { 
            g_signal_connect(button, "clicked", G_CALLBACK(on_num_clicked), (gpointer)buttons[i]);
        }

        gtk_grid_attach(GTK_GRID(grid), button, x, y, 1, 1);
    }

    gtk_widget_show_all(window);
    
    printf("=== GUI 계산기가 실행되었습니다 ===\n");
    printf("버튼을 누르면 이곳에 로그가 출력됩니다.\n\n");
    
    gtk_main();

    return 0;
}