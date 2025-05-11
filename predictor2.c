#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_ROWS 1000// Maksimum baris data

// Struktur untuk menyimpan data dari 3 kolom
typedef struct {
    double x;        // Tahun
    double y;        // Persentase pengguna internet
    double z;        // Populasi
    int missing_y;   // 1 jika Percentage_Internet_User kosong
    int missing_z;   // 1 jika Population kosong
} DataPoint;// Definisi tipe DataPoint

// Baca CSV dengan 3 kolom, simpan ke dalam array data[]
int readCSV(const char *filename, DataPoint data[]) {
    FILE *fp = fopen(filename, "r");//membuka file dengan mode read
    if (!fp) { perror("Error membuka file"); return -1; }//jika gagal membuka file maka return -1
    
    char line[1024];// array untuk menyimpan baris data
    // Baca header (asumsi ada)
    if (!fgets(line, sizeof(line), fp)) { fclose(fp); return 0; }//jika tidak ada header maka return 0

    int i = 0;// Index untuk mengisi array data
    while (fgets(line, sizeof(line), fp)) {// membaca baris demi baris sampai akhir file
        line[strcspn(line, "\r\n")] = 0;//menghapus karakter newline

        // Parse Year
        char *token = strtok(line, ",");//membagi baris menjadi token menggunakan koma sebagai pemisah
        if (token && *token != '\0') {//jika token tidak kosong
            data[i].x = atof(token);//konversi string ke double
        } else {//jika token kosong
            data[i].x = 0;//set tahun ke 0
        }

        // Parse Percentage_Internet_User
        token = strtok(NULL, ",");//membagi baris menjadi token menggunakan koma sebagai pemisah
        if (token && *token != '\0') {//jika token tidak kosong
            data[i].y = atof(token);//konversi string ke double
            data[i].missing_y = 0;//set missing_y ke 0
        } else {//jika token kosong
            data[i].y = 0;//set persentase pengguna internet ke 0
            data[i].missing_y = 1;//set missing_y ke 1
        }

        // Parse Population
        token = strtok(NULL, ",");//membagi baris menjadi token menggunakan koma sebagai pemisah
        if (token && *token != '\0') {//jika token tidak kosong
            data[i].z = atof(token);// konversi string ke double
            data[i].missing_z = 0;//set missing_z ke 0
        } else {//jika token kosong
            data[i].z = 0;//set populasi ke 0
            data[i].missing_z = 1;//set missing_z ke 1
        }

        i++;//increment index
        if (i >= MAX_ROWS) break;//jika index lebih besar dari MAX_ROWS maka break
    }
    
    fclose(fp);//tutup file
    return i;//return jumlah baris data
}

// Regresi linier sederhana untuk kolom y (Percentage_Internet_User)
void predictLinearY(DataPoint data[], double y_pred[], int n) {
    double sumx = 0, sumy = 0, sumx2 = 0, sumxy = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {//mengiterasi setiap baris data
        if (!data[i].missing_y) {//jika persentase pengguna internet tidak kosong
            sumx += data[i].x;//jumlahkan tahun
            sumy += data[i].y;//jumlahkan persentase pengguna internet
            sumx2 += data[i].x * data[i].x;//jumlahkan kuadrat tahun
            sumxy += data[i].x * data[i].y;//jumlahkan perkalian tahun dengan persentase pengguna internet
            count++;//increment count
        }
    }
    
    if (count < 2) return; // jika kurang dari 2 titik maka langsung keluar dari fungsi
    
    double d = count * sumx2 - sumx * sumx;
    if (d == 0) return;//jika d sama dengan 0 berati regresi tidak dapat dilakukan

    double m = (count * sumxy - sumx * sumy) / d;//menghitung m slope regresi
    double c = (sumy * sumx2 - sumx * sumxy) / d;//menghitung c intercept regresi

    for (int i = 0; i < n; i++) {//mengiterasi setiap baris data
        if (data[i].missing_y) {//jika persentase pengguna internet kosong
            y_pred[i] = m * data[i].x + c;//menghitung prediksi persentase pengguna internet
            if (y_pred[i] < 0) y_pred[i] = 0; // Pastikan persentase tidak negatif
        } else {//jika persentase pengguna internet tidak kosong
            y_pred[i] = data[i].y; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Regresi linier sederhana untuk kolom z (Population)
void predictLinearZ(DataPoint data[], double z_pred[], int n) {
    double sumx = 0, sumz = 0, sumx2 = 0, sumxz = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_z) {//jika populasi tidak kosong
            sumx += data[i].x;//jumlahkan tahun
            sumz += data[i].z;//jumlahkan populasi
            sumx2 += data[i].x * data[i].x;//jumlahkan kuadrat tahun
            sumxz += data[i].x * data[i].z;//jumlahkan perkalian tahun dengan populasi
            count++;//increment count
        }
    }
    
    if (count < 2) return; // Tidak bisa regresi dengan kurang dari 2 titik
    
    double d = count * sumx2 - sumx * sumx;//menghitung d
    if (d == 0) return;//jika d sama dengan 0 berati regresi tidak dapat dilakukan

    double m = (count * sumxz - sumx * sumz) / d;//menghitung m slope regresi
    double c = (sumz * sumx2 - sumx * sumxz) / d;//menghitung c intercept regresi

    for (int i = 0; i < n; i++) {//mengiterasi setiap baris data
        if (data[i].missing_z) {//jika populasi kosong
            z_pred[i] = m * data[i].x + c;//menghitung prediksi populasi
            if (z_pred[i] < 0) z_pred[i] = 0; // Pastikan populasi tidak negatif
        } else {//jika populasi tidak kosong
            z_pred[i] = data[i].z; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Interpolasi linier lokal untuk kolom y (Percentage_Internet_User)
void predictInterpolationY(DataPoint data[], double y_pred[], int n) {
    for (int i = 0; i < n; i++) {//mengiterasi setiap baris data
        if (data[i].missing_y) {//jika persentase pengguna internet kosong
            int left = -1, right = -1;//indeks tetangga kiri dan kanan

            // Cari tetangga kiri
            for (int j = i - 1; j >= 0; j--) {
                if (!data[j].missing_y) { left = j; break; }
            }

            // Cari tetangga kanan
            for (int j = i + 1; j < n; j++) {
                if (!data[j].missing_y) { right = j; break; }
            }

            //jika ada tetangga kiri dan kanan
            if (left != -1 && right != -1) {
                // Gunakan interpolasi linier lokal
                double slope = (data[right].y - data[left].y) / (data[right].x - data[left].x);
                y_pred[i] = data[left].y + slope * (data[i].x - data[left].x);
                if (y_pred[i] < 0) y_pred[i] = 0; // Pastikan persentase tidak negatif
            } else if (left != -1) {
                // Ekstrapolasi dari kiri jika tidak ada tetangga kanan
                int left2 = -1;
                for (int j = left - 1; j >= 0; j--) {
                    if (!data[j].missing_y) { left2 = j; break; }
                }
                if (left2 != -1) {
                    double slope = (data[left].y - data[left2].y) / (data[left].x - data[left2].x);
                    y_pred[i] = data[left].y + slope * (data[i].x - data[left].x);
                    if (y_pred[i] < 0) y_pred[i] = 0;
                } else {
                    y_pred[i] = data[left].y; // Gunakan nilai terakhir yang tersedia
                }
            } else if (right != -1) {
                // Ekstrapolasi dari kanan jika tidak ada tetangga kiri
                int right2 = -1;
                for (int j = right + 1; j < n; j++) {
                    if (!data[j].missing_y) { right2 = j; break; }
                }
                if (right2 != -1) {
                    double slope = (data[right2].y - data[right].y) / (data[right2].x - data[right].x);
                    y_pred[i] = data[right].y - slope * (data[right].x - data[i].x);
                    if (y_pred[i] < 0) y_pred[i] = 0;
                } else {
                    y_pred[i] = data[right].y; // Gunakan nilai pertama yang tersedia
                }
            } else {
                y_pred[i] = 0; // Default jika tidak ada data referensi
            }
        } else {
            y_pred[i] = data[i].y; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Interpolasi linier lokal untuk kolom z (Population)
void predictInterpolationZ(DataPoint data[], double z_pred[], int n) {
    for (int i = 0; i < n; i++) {
        if (data[i].missing_z) {
            int left = -1, right = -1;

            // Cari tetangga kiri
            for (int j = i - 1; j >= 0; j--) {
                if (!data[j].missing_z) { left = j; break; }
            }

            // Cari tetangga kanan
            for (int j = i + 1; j < n; j++) {
                if (!data[j].missing_z) { right = j; break; }
            }

            if (left != -1 && right != -1) {
                // Gunakan interpolasi linier lokal
                double slope = (data[right].z - data[left].z) / (data[right].x - data[left].x);
                z_pred[i] = data[left].z + slope * (data[i].x - data[left].x);
                if (z_pred[i] < 0) z_pred[i] = 0; // Pastikan populasi tidak negatif
            } else if (left != -1) {
                // Ekstrapolasi dari kiri jika tidak ada tetangga kanan
                int left2 = -1;
                for (int j = left - 1; j >= 0; j--) {
                    if (!data[j].missing_z) { left2 = j; break; }
                }
                if (left2 != -1) {
                    double slope = (data[left].z - data[left2].z) / (data[left].x - data[left2].x);
                    z_pred[i] = data[left].z + slope * (data[i].x - data[left].x);
                    if (z_pred[i] < 0) z_pred[i] = 0;
                } else {
                    z_pred[i] = data[left].z; // Gunakan nilai terakhir yang tersedia
                }
            } else if (right != -1) {
                // Ekstrapolasi dari kanan jika tidak ada tetangga kiri
                int right2 = -1;
                for (int j = right + 1; j < n; j++) {
                    if (!data[j].missing_z) { right2 = j; break; }
                }
                if (right2 != -1) {
                    double slope = (data[right2].z - data[right].z) / (data[right2].x - data[right].x);
                    z_pred[i] = data[right].z - slope * (data[right].x - data[i].x);
                    if (z_pred[i] < 0) z_pred[i] = 0;
                } else {
                    z_pred[i] = data[right].z; // Gunakan nilai pertama yang tersedia
                }
            } else {
                z_pred[i] = 0; // Default jika tidak ada data referensi
            }
        } else {
            z_pred[i] = data[i].z; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Regresi polinomial untuk kolom y (Percentage_Internet_User)
void predictPolynomialY(DataPoint data[], double y_pred[], int n, double *a0, double *a1, double *a2) {
    //variabel yang akan menyimpan pejumlahan statistik
    double sx = 0, sy = 0, sx2 = 0, sx3 = 0, sx4 = 0, sxy = 0, sx2y = 0;
    int count = 0;
    
    //perhitungan pejumlahan statistik
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_y) {//mengecek data yang tidak missing
            double xi = data[i].x, yi = data[i].y;//ambil nilai x dan y dari data tersebut
            sx += xi; sy += yi;//sx bertambah dengan nilai x ke i , sy bertambah dengan nilai y ke i
            sx2 += xi * xi;//tambahkan nilai x^2 ke sx2 membentuk sigma x^2
            sx3 += xi * xi * xi;//tambahkan nilai x^3 ke sx3 membentuk sigma x^3
            sx4 += xi * xi * xi * xi;//tambahkan nilai x^4 ke sx4 membentuk sigma x^4
            sxy += xi * yi;//tambahkan nilai xy ke sxy membentuk sigma xy
            sx2y += xi * xi * yi;//tambahkan nilai x^2y ke sx2y membentuk sigma x^2y
            count++;//hitung jumlah data
        }
    }

    if (count < 3) return;//jika data kurang dari 3, maka tidak dilakukan regresi

    // Matriks augmented 3x4 untuk persamaan normal
    double M[3][4] = {//mendefinisikan array 2 dimensi dengan 3 baris dan 4 kolom
        { (double)count, sx,   sx2,  sy   },//persamaan metode least square
        { sx,            sx2,  sx3,  sxy  },//persamaan kedua metode least square
        { sx2,           sx3,  sx4,  sx2y }//persamaan ketiga metode least square
    };

    // Eliminasi Gauss
    for (int k = 0; k < 3; k++) {//perhitungan eliminasi gauss
        double pivot = M[k][k];//mencari pivot
        if (fabs(pivot) < 1e-10) continue;//jika pivot mendekati 0, maka lanjutkan ke iterasi selanjutnya

        for (int j = k; j < 4; j++) {//perhitungan eliminasi gauss
            M[k][j] /= pivot;//menormalisasikan baris pivot
        }

        for (int i = 0; i < 3; i++) {//perhitungan eliminasi gauss
            if (i != k) {//jika bukan baris pivot
                double factor = M[i][k];//mencari faktor
                for (int j = k; j < 4; j++) {//perhitungan eliminasi gauss
                    M[i][j] -= factor * M[k][j];//mengurangi baris dengan faktor
                }
            }
        }
    }

    *a0 = M[0][3];//mengambil nilai a0, a1, dan a2 dari matriks augmented
    *a1 = M[1][3];//mengambil nilai a0, a1, dan a2 dari matriks augmented
    *a2 = M[2][3];//mengambil nilai a0, a1, dan a2 dari matriks augmented

    for (int i = 0; i < n; i++) {//iterasi setiap baris data
        if (data[i].missing_y) {//jika persentase pengguna internet kosong
            double xi = data[i].x;//ambil nilai x
            y_pred[i] = *a0 + *a1 * xi + *a2 * xi * xi;//hitung persentase pengguna internet
            if (y_pred[i] < 0) y_pred[i] = 0; // Pastikan persentase tidak negatif
        } else {//jika persentase pengguna internet tidak kosong
            y_pred[i] = data[i].y; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Regresi polinomial untuk kolom z (Population)
void predictPolynomialZ(DataPoint data[], double z_pred[], int n, double *a0, double *a1, double *a2) {
    double sx = 0, sz = 0, sx2 = 0, sx3 = 0, sx4 = 0, sxz = 0, sx2z = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {//loop setiap baris data
        if (!data[i].missing_z) {//jika persentase populasi tidak kosong
            double xi = data[i].x, zi = data[i].z;//ambil nilai x dan z
            //update semua statistik yang dibutuhkan
            sx += xi; sz += zi;
            sx2 += xi * xi;
            sx3 += xi * xi * xi;
            sx4 += xi * xi * xi * xi;
            sxz += xi * zi;
            sx2z += xi * xi * zi;
            count++;
        }
    }

    if (count < 3) return;//jika data kurang dari 3 maka data tidak cukup untuk melakukan regresi

    // Matriks augmented 3x4 untuk persamaan normal
    double M[3][4] = {
        { (double)count, sx,   sx2,  sz   },
        { sx,            sx2,  sx3,  sxz  },
        { sx2,           sx3,  sx4,  sx2z }
    };

    // Eliminasi Gauss
    for (int k = 0; k < 3; k++) {//loop eliminasi gauss
        double pivot = M[k][k];//menimpan nilai pivot
        if (fabs(pivot) < 1e-10) continue;//jika pivot mendekati 0, maka lanjutkan ke iterasi selanjutnya

        for (int j = k; j < 4; j++) {//loop eliminasi gauss
            M[k][j] /= pivot;//menormalisasikan baris pivot
        }

        for (int i = 0; i < 3; i++) {//loop kedua eliminasi gauss
            if (i != k) {//jika bukan baris pivot
                double factor = M[i][k];//menyimpan faktor eliminasi
                for (int j = k; j < 4; j++) {
                    M[i][j] -= factor * M[k][j];//eliminasi baris ke i di elemen k
                }
            }
        }
    }

    *a0 = M[0][3];//menetapkan elemen matriks terakhir pada baris pertama
    *a1 = M[1][3];//menetapkan elemen matriks terakhir pada baris kedua
    *a2 = M[2][3];//menetapkan elemen matriks terakhir pada baris ketiga

    for (int i = 0; i < n; i++) {//loop untuk mengiterasi setiap baris data
        if (data[i].missing_z) {//jika persentase populasi kosong
            double xi = data[i].x;//maka ambil nilai x
            z_pred[i] = *a0 + *a1 * xi + *a2 * xi * xi;//hitung persentase populasi
            if (z_pred[i] < 0) z_pred[i] = 0; // Pastikan populasi tidak negatif
        } else {
            z_pred[i] = data[i].z; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Tulis hasil ke file baru
void writeCSV(const char *filename, DataPoint data[], double y_pred[], double z_pred[], int n) {
    FILE *fp = fopen(filename, "w");//membuka file dengan mode write
    if (!fp) { perror("Error membuka file untuk menulis"); return; }//jika gagal membuka file maka return error message
    
    fprintf(fp, "Year,Percentage_Internet_User,Population\n");//menulis header
    
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%.0f,", data[i].x); // year (tanpa desimal)
        
        if (data[i].missing_y) {//jika persentase pengguna internet kosong
            fprintf(fp, "%.6f,", y_pred[i]); // maka gunakan prediksi
        } else {//jika persentase pengguna internet tidak kosong
            fprintf(fp, "%.6f,", data[i].y); // maka gunakan data asli
        }
        
        if (data[i].missing_z) {//jika persentase populasi kosong
            fprintf(fp, "%.0f\n", z_pred[i]); // maka gunakan prediksi
        } else {//jika persentase populasi tidak kosong
            fprintf(fp, "%.0f\n", data[i].z); // maka gunakan data asli
        }
    }
    
    fclose(fp);//tutup file
}

// Plot data asli vs prediksi untuk y (Percentage_Internet_User)
void plotDataY(DataPoint data[], double y_pred[], int n, const char *title) {
    FILE *gp = popen("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\" -p", "w");//membuka gnuplot
    if (!gp) { perror("Error membuka gnuplot"); return; }//jika gagal membuka gnuplot maka return error message
    
    fprintf(gp, "set title \"%s\"\n", title);
    fprintf(gp, "set xlabel \"Year\"\n");
    fprintf(gp, "set ylabel \"Percentage Internet User\"\n");
    fprintf(gp, "plot '-' with points pt 7 lc rgb 'blue' title 'Data Asli', \\\n");
    fprintf(gp, "     '-' with points pt 5 lc rgb 'red'  title 'Prediksi'\n");

    // Data asli
    for (int i = 0; i < n; i++) {//loop setiap baris data
        if (!data[i].missing_y) {//jika persentase pengguna internet tidak kosong
            fprintf(gp, "%g %g\n", data[i].x, data[i].y);//maka tampilkan data asli
        }
    }
    fprintf(gp, "e\n");//tampilkan data asli

    // Data prediksi
    for (int i = 0; i < n; i++) {//loop setiap baris data
        if (data[i].missing_y) {//jika persentase pengguna internet kosong
            fprintf(gp, "%g %g\n", data[i].x, y_pred[i]);//maka tampilkan prediksi
        }
    }
    fprintf(gp, "e\n");//tampilkan prediksi

    pclose(gp);//tutup gnuplot
}

// Plot data asli vs prediksi untuk z (Population)
void plotDataZ(DataPoint data[], double z_pred[], int n, const char *title) {
    FILE *gp = popen("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\" -p", "w");//membuka gnuplot
    if (!gp) { perror("Error membuka gnuplot"); return; }//jika gagal membuka gnuplot maka return error message
    
    fprintf(gp, "set title \"%s\"\n", title);
    fprintf(gp, "set xlabel \"Year\"\n");
    fprintf(gp, "set ylabel \"Population\"\n");
    fprintf(gp, "plot '-' with points pt 7 lc rgb 'blue' title 'Data Asli', \\\n");
    fprintf(gp, "     '-' with points pt 5 lc rgb 'red'  title 'Prediksi'\n");

    // Data asli
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_z) {//jika persentase populasi tidak kosong
            fprintf(gp, "%g %g\n", data[i].x, data[i].z);//maka tampilkan data asli
        }
    }
    fprintf(gp, "e\n");//tutup plot

    // Data prediksi
    for (int i = 0; i < n; i++) {
        if (data[i].missing_z) {//jika persentase populasi kosong
            fprintf(gp, "%g %g\n", data[i].x, z_pred[i]);//maka tampilkan prediksi
        }
    }
    fprintf(gp, "e\n");

    pclose(gp);//tutup gnuplot
}

// Fungsi untuk mencari nilai prediksi berdasarkan tahun
double findPredictionForYear(DataPoint data[], double pred[], int n, int year) {
    for (int i = 0; i < n; i++) {//mengiterasi setiap baris data
        if (data[i].x == year) {//jika data tahun sama dengan tahun
            return pred[i];//maka kembalikan prediksi
        }
    }
    return -1; // Tahun tidak ditemukan
}

// Fungsi untuk menghitung prediksi untuk tahun di luar data
double calculatePredictionForYear(double a0, double a1, double a2, int year) {
    double result = a0 + a1 * year + a2 * year * year;//menggunakan persamaan polinomial
    return (result < 0) ? 0 : result; // Pastikan hasilnya tidak negatif
}

// Fungsi untuk menampilkan persamaan polinomial
void displayPolynomialEquation(double a0, double a1, double a2) {
    printf("y = ");//menampilkan persamaan polinomial
    
    if (fabs(a2) > 1e-10) {//menegecek apakah a2 cukup besar untuk ditampilkan
        printf("%g x^2 ", a2);//maka tampilkan persamaan polinomial
        if (a1 >= 0) printf("+ ");//menambahkan tanda '+' jika a1 positif
    }
    
    if (fabs(a1) > 1e-10) {//menegecek apakah a1 cukup besar untuk ditampilkan
        printf("%g x ", a1);//maka tampilkan persamaan polinomial
        if (a0 >= 0) printf("+ ");//menambahkan tanda '+' jika a0 positif
    }
    
    if (fabs(a0) > 1e-10 || (fabs(a1) < 1e-10 && fabs(a2) < 1e-10)) {//menegecek apakah a0 cukup besar untuk ditampilkan
        printf("%g", a0);//maka tampilkan persamaan polinomial
    }
    
    printf("\n");
}

int main(int argc, char **argv) {
    const char *input_file = (argc > 1) ? argv[1] : "data.csv";//memeriksa apakah argc lebih besar dari 1, jika iya maka argv[1] akan digunakan sebagai input_file
    DataPoint data[MAX_ROWS];//deklarasi array data
    
    int n = readCSV(input_file, data);//membaca data dari file
    if (n <= 0) { printf("Tidak ada data.\n"); return 1; }//jika tidak ada data maka return 1
    
    printf("Berhasil membaca %d baris data.\n", n);//menampilkan jumlah baris data

    // Array untuk menyimpan prediksi
    double y_pred_lin[MAX_ROWS] = {0};
    double y_pred_interp[MAX_ROWS] = {0};
    double y_pred_poly[MAX_ROWS] = {0};
    double z_pred_lin[MAX_ROWS] = {0};
    double z_pred_interp[MAX_ROWS] = {0};
    double z_pred_poly[MAX_ROWS] = {0};
    
    // Koefisien persamaan polinomial
    double a0_y = 0, a1_y = 0, a2_y = 0; // Untuk Percentage_Internet_User
    double a0_z = 0, a1_z = 0, a2_z = 0; // Untuk Population

    // Prediksi dengan berbagai metode
    printf("Melakukan prediksi dengan regresi linier...\n");
    predictLinearY(data, y_pred_lin, n);
    predictLinearZ(data, z_pred_lin, n);
    
    printf("Melakukan prediksi dengan interpolasi linier...\n");
    predictInterpolationY(data, y_pred_interp, n);
    predictInterpolationZ(data, z_pred_interp, n);
    
    printf("Melakukan prediksi dengan regresi polinomial...\n");
    predictPolynomialY(data, y_pred_poly, n, &a0_y, &a1_y, &a2_y);
    predictPolynomialZ(data, z_pred_poly, n, &a0_z, &a1_z, &a2_z);

    // Tulis hasil prediksi ke file
    writeCSV("prediksi_linear.csv", data, y_pred_lin, z_pred_lin, n);
    writeCSV("prediksi_interpolasi.csv", data, y_pred_interp, z_pred_interp, n);
    writeCSV("prediksi_polinomial.csv", data, y_pred_poly, z_pred_poly, n);

    // Plot hasil prediksi
    printf("Membuat grafik hasil prediksi...\n");
    plotDataY(data, y_pred_lin, n, "Regresi Linier - Percentage Internet User");
    plotDataZ(data, z_pred_lin, n, "Regresi Linier - Population");
    plotDataY(data, y_pred_interp, n, "Interpolasi Linier - Percentage Internet User");
    plotDataZ(data, z_pred_interp, n, "Interpolasi Linier - Population");
    plotDataY(data, y_pred_poly, n, "Regresi Polinomial - Percentage Internet User");
    plotDataZ(data, z_pred_poly, n, "Regresi Polinomial - Population");

    printf("\n===== HASIL PREDIKSI NILAI YANG HILANG =====\n\n");
    
    // 1. Perkiraan nilai yang hilang untuk tahun-tahun yang diminta
    printf("1. Perkiraan nilai yang hilang:\n");
    printf("   a. Jumlah penduduk Indonesia di tahun 2005: %.0f\n", findPredictionForYear(data, z_pred_poly, n, 2005));
    printf("   b. Jumlah penduduk Indonesia di tahun 2006: %.0f\n", findPredictionForYear(data, z_pred_poly, n, 2006));
    printf("   c. Jumlah penduduk Indonesia di tahun 2015: %.0f\n", findPredictionForYear(data, z_pred_poly, n, 2015));
    printf("   d. Jumlah penduduk Indonesia di tahun 2016: %.0f\n", findPredictionForYear(data, z_pred_poly, n, 2016));
    printf("   e. Persentase pengguna Internet Indonesia di tahun 2005: %.6f%%\n", findPredictionForYear(data, y_pred_poly, n, 2005));
    printf("   f. Persentase pengguna Internet Indonesia di tahun 2006: %.6f%%\n", findPredictionForYear(data, y_pred_poly, n, 2006));
    printf("   g. Persentase pengguna Internet Indonesia di tahun 2015: %.6f%%\n", findPredictionForYear(data, y_pred_poly, n, 2015));
    printf("   h. Persentase pengguna Internet Indonesia di tahun 2016: %.6f%%\n", findPredictionForYear(data, y_pred_poly, n, 2016));

    // 2. Persamaan polinomial untuk kedua data
    printf("\n2. Persamaan polinomial:\n");
    printf("   a. Persentase pengguna Internet Indonesia: ");
    displayPolynomialEquation(a0_y, a1_y, a2_y);
    printf("   b. Pertumbuhan populasi Indonesia: ");
    displayPolynomialEquation(a0_z, a1_z, a2_z);

    // 3. Estimasi untuk tahun 2030 dan 2035
    printf("\n3. Estimasi:\n");
    printf("   a. Jumlah populasi Indonesia di tahun 2030: %.0f\n", calculatePredictionForYear(a0_z, a1_z, a2_z, 2030));
    printf("   b. Jumlah pengguna Internet di Indonesia di tahun 2035: %.6f%%\n", calculatePredictionForYear(a0_y, a1_y, a2_y, 2035));
    
    // Menghitung jumlah pengguna Internet (persentase * populasi / 100)
    double population_2035 = calculatePredictionForYear(a0_z, a1_z, a2_z, 2035);
    double percentage_2035 = calculatePredictionForYear(a0_y, a1_y, a2_y, 2035);
    printf("      Jumlah pengguna Internet dalam angka di tahun 2035: %.0f orang\n", 
           population_2035 * percentage_2035 / 100.0);
    return 0;
}