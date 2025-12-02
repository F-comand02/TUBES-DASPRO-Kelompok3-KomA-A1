#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RESET   "\x1b[0m"
#define BLUE    "\x1b[94m"
#define CYAN    "\x1b[96m"
#define GREEN   "\x1b[92m"
#define YELLOW  "\x1b[93m"
#define RED     "\x1b[91m"
#define WHITE   "\x1b[97m"

#define KAMAR_FILE "kamar.txt"         
#define RESERVASI_FILE "reservasi.txt"

#define MAX_KAMAR 10
#define MAX_RESERVASI 50

typedef struct {
    int nomor;
    char tipe[20];
    int tersedia;   
    int harga;      
} Kamar;

typedef struct {
    int id_reservasi;
    int nomor_kamar;
    char nama_pemesan[50];
    char tgl_checkin[15];
    char tgl_checkout[15];
} Reservasi;


Kamar daftarKamar[MAX_KAMAR];
Reservasi daftarReservasi[MAX_RESERVASI];
int jumlahReservasi = 0;


void inisialisasiDataKamar(void);
void tampilkanMenu(void);
void carikamar(void);
void buatReservasiBaru(void);
void tampilkanDaftarReservasi(void);
void prosesCheckOut(void);
void bersihkanBuffer(void);
void muatDataReservasi(void);
void simpanDataReservasi(void);
int validasiTanggal(const char *tgl);
int hitungHari(const char *checkin, const char *checkout);
float getHargaKamar(int nomorKamar);
char* getTipeKamar(int nomorKamar);
void tampilkantable_reservasi_single(const Reservasi *r);
void cetakStrukFileReservasi(const Reservasi *r, int saat); 

void bersihkanBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void inisialisasiDataKamar(void) {
    for (int i = 0; i < MAX_KAMAR; i++) {
        daftarKamar[i].nomor = 101 + i;
        if (i < 3) {
            strcpy(daftarKamar[i].tipe, "Standard");
            daftarKamar[i].harga = 300000;
        } else if (i < 7) {
            strcpy(daftarKamar[i].tipe, "Deluxe");
            daftarKamar[i].harga = 500000;
        } else {
            strcpy(daftarKamar[i].tipe, "Suite");
            daftarKamar[i].harga = 800000;
        }
        daftarKamar[i].tersedia = 1;
    }
}

void muatDataReservasi(void) {
    FILE *f = fopen(RESERVASI_FILE, "r");
    if (!f) {
        printf(YELLOW "[INFO] File reservasi tidak ditemukan. File akan dibuat saat menyimpan.\n" RESET);
        return;
    }

    Reservasi temp;
    jumlahReservasi = 0;
    while (fscanf(f, "%d;%d;%49[^;];%14[^;];%14[^\n]\n",
                  &temp.id_reservasi,
                  &temp.nomor_kamar,
                  temp.nama_pemesan,
                  temp.tgl_checkin,
                  temp.tgl_checkout) == 5 &&
           jumlahReservasi < MAX_RESERVASI) {

        daftarReservasi[jumlahReservasi] = temp;

        for (int i = 0; i < MAX_KAMAR; i++) {
            if (daftarKamar[i].nomor == temp.nomor_kamar) {
                daftarKamar[i].tersedia = 0;
                break;
            }
        }

        jumlahReservasi++;
    }

    fclose(f);

    if (jumlahReservasi > 0)
        printf(GREEN "[INFO] %d data reservasi dimuat dari '%s'.\n" RESET, jumlahReservasi, RESERVASI_FILE);
    else
        printf(YELLOW "[INFO] Tidak ada data reservasi pada file.\n" RESET);
}

void simpanDataReservasi(void) {
    FILE *f = fopen(RESERVASI_FILE, "w");
    if (!f) {
        printf(RED "[ERROR] Gagal membuka file '%s' untuk menyimpan.\n" RESET, RESERVASI_FILE);
        return;
    }

    for (int i = 0; i < jumlahReservasi; i++) {
        fprintf(f, "%d;%d;%s;%s;%s\n",
                daftarReservasi[i].id_reservasi,
                daftarReservasi[i].nomor_kamar,
                daftarReservasi[i].nama_pemesan,
                daftarReservasi[i].tgl_checkin,
                daftarReservasi[i].tgl_checkout);
    }

    fclose(f);
    printf(GREEN "[INFO] Database reservasi disimpan ke '%s'.\n" RESET, RESERVASI_FILE);
}

void tampilkanMenu(void) {
    printf(BLUE "=====================================================\n" RESET);
    printf(BLUE "                SISTEM RESERVASI HOTEL               \n" RESET);
    printf(BLUE "=====================================================\n" RESET);

    printf(YELLOW "1. Cari Kamar Tersedia\n" RESET);
    printf(YELLOW "2. Buat Reservasi Baru\n" RESET);
    printf(YELLOW "3. Tampilkan Daftar Reservasi\n" RESET);
    printf(YELLOW "4. Check-out\n" RESET);
    printf(YELLOW "5. Simpan & Keluar\n" RESET);

    printf(BLUE "-----------------------------------------------------\n" RESET);
}

void carikamar(void) {
    printf(CYAN "\n--- DAFTAR KAMAR TERSEDIA ---\n" RESET);

    int ada = 0;
    for (int i = 0; i < MAX_KAMAR; i++) {
        if (daftarKamar[i].tersedia) {
            printf("Kamar %d | %-7s | Harga: Rp %d\n",
                   daftarKamar[i].nomor,
                   daftarKamar[i].tipe,
                   daftarKamar[i].harga);
            ada = 1;
        }
    }
    if (!ada) {
        printf(RED "[ERROR] Tidak ada kamar tersedia.\n" RESET);
    }
}

int validasiTanggal(const char *tgl) {
    int d, m, y;
    if (sscanf(tgl, "%d/%d/%d", &d, &m, &y) != 3)
        return 0;
    if (y < 1900 || m < 1 || m > 12 || d < 1) return 0;

    int hariBulan[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if ((y % 400 == 0) || (y % 4 == 0 && y % 100 != 0)) hariBulan[1] = 29;
    if (d > hariBulan[m-1]) return 0;
    return 1;
}

int hitungHari(const char *checkin, const char *checkout) {
    int d1, m1, y1, d2, m2, y2;
    if (sscanf(checkin, "%d/%d/%d", &d1, &m1, &y1) != 3) return 0;
    if (sscanf(checkout, "%d/%d/%d", &d2, &m2, &y2) != 3) return 0;

    struct tm t1 = {0}, t2 = {0};
    t1.tm_mday = d1; t1.tm_mon = m1 - 1; t1.tm_year = y1 - 1900;
    t2.tm_mday = d2; t2.tm_mon = m2 - 1; t2.tm_year = y2 - 1900;

    time_t time1 = mktime(&t1);
    time_t time2 = mktime(&t2);
    if (time1 == (time_t)-1 || time2 == (time_t)-1) return 0;

    double diff = difftime(time2, time1) / (60.0 * 60.0 * 24.0);
    return (int) diff;
}

float getHargaKamar(int nomorKamar) {
    for (int i = 0; i < MAX_KAMAR; i++) {
        if (daftarKamar[i].nomor == nomorKamar) return daftarKamar[i].harga;
    }
    return 0;
}

char* getTipeKamar(int nomorKamar) {
    static char unknown[] = "Tidak diketahui";
    for (int i = 0; i < MAX_KAMAR; i++) {
        if (daftarKamar[i].nomor == nomorKamar) return daftarKamar[i].tipe;
    }
    return unknown;
}

void tampilkantable_reservasi_single(const Reservasi *r) {
    int lama = hitungHari(r->tgl_checkin, r->tgl_checkout);
    int harga = (int)getHargaKamar(r->nomor_kamar);
    int total = harga * lama;
    char *tipe = getTipeKamar(r->nomor_kamar);

    printf(YELLOW "----------------------------------------\n" RESET);
    printf("ID: %d\n", r->id_reservasi);
    printf("Kamar: %d (%s)\n", r->nomor_kamar, tipe);
    printf("Harga per malam : Rp %d\n", harga);
    printf("Lama menginap   : %d hari\n", lama);
    printf("Total harga     : Rp %d\n", total);
    printf("Nama            : %s\n", r->nama_pemesan);
    printf("Check-in: %s | Check-out: %s\n",
           r->tgl_checkin, r->tgl_checkout);
}

void cetakStrukFileReservasi(const Reservasi *r, int saat) {
    int lama = hitungHari(r->tgl_checkin, r->tgl_checkout);
    int harga = (int)getHargaKamar(r->nomor_kamar);
    int total = harga * lama;
    char *tipe = getTipeKamar(r->nomor_kamar);

    char filename[128];
    if (saat == 0)
        snprintf(filename, sizeof(filename), "struk_%d.txt", r->id_reservasi);
    else
        snprintf(filename, sizeof(filename), "struk_%d_checkout.txt", r->id_reservasi);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf(RED "[ERROR] Gagal membuat file struk '%s'.\n" RESET, filename);
        return;
    }

    fprintf(fp, "=========================================================\n");
    if (saat == 0)
        fprintf(fp, "                     STRUK RESERVASI                    \n");
    else
        fprintf(fp, "                      STRUK CHECK-OUT                    \n");
    fprintf(fp, "=========================================================\n");
    fprintf(fp, "| %-15s | %-25s |\n", "Data", "Keterangan");
    fprintf(fp, "---------------------------------------------------------\n");
    fprintf(fp, "| %-15s | %-25s |\n", "ID Reservasi", "");
    fprintf(fp, "| %-15s | %-25d |\n", "ID Reservasi", r->id_reservasi);
    fprintf(fp, "| %-15s | %-25s |\n", "Nama Tamu", r->nama_pemesan);
    fprintf(fp, "| %-15s | %-25s |\n", "Check-in", r->tgl_checkin);
    fprintf(fp, "| %-15s | %-25s |\n", "Check-out", r->tgl_checkout);
    fprintf(fp, "| %-15s | %-25s |\n", "Tipe Kamar", tipe);
    fprintf(fp, "| %-15s | %-25d |\n", "Harga / Malam", harga);
    fprintf(fp, "| %-15s | %-25d |\n", "Lama Inap", lama);
    fprintf(fp, "| %-15s | Rp %-21d |\n", "Total Bayar", total);
    fprintf(fp, "=========================================================\n");

    fclose(fp);
    printf(GREEN "[OK] Struk tercetak: %s\n" RESET, filename);
}

void buatReservasiBaru(void) {
    printf(CYAN "\n--- BUAT RESERVASI BARU ---\n" RESET);

    if (jumlahReservasi >= MAX_RESERVASI) {
        printf(RED "[ERROR] Kapasitas reservasi penuh.\n" RESET);
        return;
    }

    carikamar();

    int noKamar;
    printf("\nMasukkan nomor kamar yang diinginkan: ");
    if (scanf("%d", &noKamar) != 1) {
        printf(RED "[ERROR] Input nomor kamar tidak valid.\n" RESET);
        bersihkanBuffer();
        return;
    }
    bersihkanBuffer();

    int idxK = -1;
    for (int i = 0; i < MAX_KAMAR; i++) {
        if (daftarKamar[i].nomor == noKamar) { idxK = i; break; }
    }
    if (idxK == -1) {
        printf(RED "[ERROR] Nomor kamar tidak ditemukan.\n" RESET);
        return;
    }
    if (!daftarKamar[idxK].tersedia) {
        printf(RED "[ERROR] Kamar sudah terpesan.\n" RESET);
        return;
    }

    Reservasi r;
    r.nomor_kamar = noKamar;
    r.id_reservasi = 1000 + jumlahReservasi; 

    printf("Nama Pemesan: ");
    fgets(r.nama_pemesan, sizeof(r.nama_pemesan), stdin);
    r.nama_pemesan[strcspn(r.nama_pemesan, "\n")] = '\0';

   
    do {
        printf("Tanggal Check-in (dd/mm/yyyy): ");
        fgets(r.tgl_checkin, sizeof(r.tgl_checkin), stdin);
        r.tgl_checkin[strcspn(r.tgl_checkin, "\n")] = '\0';
        if (!validasiTanggal(r.tgl_checkin)) {
            printf(RED "Format/tanggal check-in tidak valid. Coba lagi.\n" RESET);
        } else break;
    } while (1);

    do {
        printf("Tanggal Check-out (dd/mm/yyyy): ");
        fgets(r.tgl_checkout, sizeof(r.tgl_checkout), stdin);
        r.tgl_checkout[strcspn(r.tgl_checkout, "\n")] = '\0';
        if (!validasiTanggal(r.tgl_checkout)) {
            printf(RED "Format/tanggal check-out tidak valid. Coba lagi.\n" RESET);
        } else break;
    } while (1);

    int lama = hitungHari(r.tgl_checkin, r.tgl_checkout);
    if (lama <= 0) {
        printf(RED "[ERROR] Tanggal check-out harus lebih besar dari check-in.\n" RESET);
        return;
    }

    
    daftarReservasi[jumlahReservasi] = r;
    jumlahReservasi++;
    daftarKamar[idxK].tersedia = 0;

    simpanDataReservasi();

    printf(GREEN "\nReservasi berhasil dibuat! ID: %d\n" RESET, r.id_reservasi);

    
    printf("\n=========================================================\n");
    printf("| %-15s | %-25s |\n", "Data", "Keterangan");
    printf("---------------------------------------------------------\n");
    printf("| %-15s | %-25d |\n", "ID Reservasi", r.id_reservasi);
    printf("| %-15s | %-25s |\n", "Nama Tamu", r.nama_pemesan);
    printf("| %-15s | %-25s |\n", "Check-in", r.tgl_checkin);
    printf("| %-15s | %-25s |\n", "Check-out", r.tgl_checkout);
    printf("| %-15s | %-25s |\n", "Tipe Kamar", getTipeKamar(r.nomor_kamar));
    printf("| %-15s | Rp %-21d |\n", "Harga / Malam", daftarKamar[idxK].harga);
    printf("| %-15s | %-25d |\n", "Lama Inap", lama);
    printf("=========================================================\n");

    
    cetakStrukFileReservasi(&r, 0);
}


void tampilkanDaftarReservasi(void) {
    printf(CYAN "\n--- DAFTAR RESERVASI ---\n" RESET);
    if (jumlahReservasi == 0) {
        printf(RED "[INFO] Belum ada reservasi.\n" RESET);
        return;
    }

    for (int i = 0; i < jumlahReservasi; i++) {
        tampilkantable_reservasi_single(&daftarReservasi[i]);
    }
    printf(YELLOW "----------------------------------------\n" RESET);
}

void prosesCheckOut(void) {
    printf(CYAN "\n--- CHECK-OUT ---\n" RESET);
    if (jumlahReservasi == 0) {
        printf(RED "[ERROR] Tidak ada reservasi.\n" RESET);
        return;
    }
    int id;
    printf("Masukkan ID Reservasi: ");
    if (scanf("%d", &id) != 1) {
        printf(RED "[ERROR] Input ID tidak valid.\n" RESET);
        bersihkanBuffer();
        return;
    }
    bersihkanBuffer();

    int idx = -1;
    for (int i = 0; i < jumlahReservasi; i++) {
        if (daftarReservasi[i].id_reservasi == id) { idx = i; break; }
    }
    if (idx == -1) {
        printf(RED "[ERROR] ID reservasi tidak ditemukan.\n" RESET);
        return;
    }

    Reservasi r = daftarReservasi[idx];
    printf(CYAN "Reservasi ditemukan: %s (Kamar %d)\n" RESET, r.nama_pemesan, r.nomor_kamar);
    printf("Aksi (C = Check-out, X = Batal): ");
    char pilih;
    if (scanf(" %c", &pilih) != 1) { bersihkanBuffer(); return; }
    bersihkanBuffer();

    if (pilih == 'C' || pilih == 'c') {
        
        cetakStrukFileReservasi(&r, 1);

        
        for (int i = 0; i < MAX_KAMAR; i++) {
            if (daftarKamar[i].nomor == r.nomor_kamar) {
                daftarKamar[i].tersedia = 1;
                break;
            }
        }

        
        for (int i = idx; i < jumlahReservasi - 1; i++) {
            daftarReservasi[i] = daftarReservasi[i + 1];
        }
        jumlahReservasi--;
        simpanDataReservasi();

        printf(GREEN "Check-out berhasil. Kamar %d sekarang tersedia.\n" RESET, r.nomor_kamar);
    } else {
        printf(YELLOW "Aksi dibatalkan.\n" RESET);
    }
}


int main(void) {
    inisialisasiDataKamar();
    muatDataReservasi();

    int pilihan;
    do {
        tampilkanMenu();
        printf(WHITE "Masukkan pilihan Anda: " RESET);
        if (scanf("%d", &pilihan) != 1) {
            printf(RED "[ERROR] Input tidak valid. Masukkan angka.\n" RESET);
            bersihkanBuffer();
            pilihan = 0;
            continue;
        }
        bersihkanBuffer();

        switch (pilihan) {
            case 1: carikamar(); break;
            case 2: buatReservasiBaru(); break;
            case 3: tampilkanDaftarReservasi(); break;
            case 4: prosesCheckOut(); break;
            case 5:
                simpanDataReservasi();
                printf(GREEN "\nProgram selesai. Terima kasih.\n" RESET);
                break;
            default:
                printf(RED "Pilihan tidak valid. Coba lagi.\n" RESET);
        }
        printf("\n");
    } while (pilihan != 5);

    return 0;
}