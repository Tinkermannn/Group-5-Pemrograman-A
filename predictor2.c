#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_ROWS 1000

// Struktur untuk menyimpan data dari 3 kolom
typedef struct {
    double x;        // Year
    double y;        // Percentage_Internet_User
    double z;        // Population
    int missing_y;   // 1 jika Percentage_Internet_User kosong
    int missing_z;   // 1 jika Population kosong
} DataPoint;

// Baca CSV dengan 3 kolom, simpan ke dalam array data[]
int readCSV(const char *filename, DataPoint data[]) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("Error membuka file"); return -1; }
    
    char line[1024];
    // Baca header (asumsi ada)
    if (!fgets(line, sizeof(line), fp)) { fclose(fp); return 0; }

    int i = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;

        // Parse Year
        char *token = strtok(line, ",");
        if (token && *token != '\0') {
            data[i].x = atof(token);
        } else {
            data[i].x = 0;
        }

        // Parse Percentage_Internet_User
        token = strtok(NULL, ",");
        if (token && *token != '\0') {
            data[i].y = atof(token);
            data[i].missing_y = 0;
        } else {
            data[i].y = 0;
            data[i].missing_y = 1;
        }

        // Parse Population
        token = strtok(NULL, ",");
        if (token && *token != '\0') {
            data[i].z = atof(token);
            data[i].missing_z = 0;
        } else {
            data[i].z = 0;
            data[i].missing_z = 1;
        }

        i++;
        if (i >= MAX_ROWS) break;
    }
    
    fclose(fp);
    return i;
}

// Regresi linier sederhana untuk kolom y (Percentage_Internet_User)
void predictLinearY(DataPoint data[], double y_pred[], int n) {
    double sumx = 0, sumy = 0, sumx2 = 0, sumxy = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_y) {
            sumx += data[i].x;
            sumy += data[i].y;
            sumx2 += data[i].x * data[i].x;
            sumxy += data[i].x * data[i].y;
            count++;
        }
    }
    
    if (count < 2) return; // Tidak bisa regresi dengan kurang dari 2 titik
    
    double d = count * sumx2 - sumx * sumx;
    if (d == 0) return;

    double m = (count * sumxy - sumx * sumy) / d;
    double c = (sumy * sumx2 - sumx * sumxy) / d;

    for (int i = 0; i < n; i++) {
        if (data[i].missing_y) {
            y_pred[i] = m * data[i].x + c;
            if (y_pred[i] < 0) y_pred[i] = 0; // Pastikan persentase tidak negatif
        } else {
            y_pred[i] = data[i].y; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Regresi linier sederhana untuk kolom z (Population)
void predictLinearZ(DataPoint data[], double z_pred[], int n) {
    double sumx = 0, sumz = 0, sumx2 = 0, sumxz = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_z) {
            sumx += data[i].x;
            sumz += data[i].z;
            sumx2 += data[i].x * data[i].x;
            sumxz += data[i].x * data[i].z;
            count++;
        }
    }
    
    if (count < 2) return; // Tidak bisa regresi dengan kurang dari 2 titik
    
    double d = count * sumx2 - sumx * sumx;
    if (d == 0) return;

    double m = (count * sumxz - sumx * sumz) / d;
    double c = (sumz * sumx2 - sumx * sumxz) / d;

    for (int i = 0; i < n; i++) {
        if (data[i].missing_z) {
            z_pred[i] = m * data[i].x + c;
            if (z_pred[i] < 0) z_pred[i] = 0; // Pastikan populasi tidak negatif
        } else {
            z_pred[i] = data[i].z; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Interpolasi linier lokal untuk kolom y (Percentage_Internet_User)
void predictInterpolationY(DataPoint data[], double y_pred[], int n) {
    for (int i = 0; i < n; i++) {
        if (data[i].missing_y) {
            int left = -1, right = -1;

            // Cari tetangga kiri
            for (int j = i - 1; j >= 0; j--) {
                if (!data[j].missing_y) { left = j; break; }
            }

            // Cari tetangga kanan
            for (int j = i + 1; j < n; j++) {
                if (!data[j].missing_y) { right = j; break; }
            }

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
    double sx = 0, sy = 0, sx2 = 0, sx3 = 0, sx4 = 0, sxy = 0, sx2y = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_y) {
            double xi = data[i].x, yi = data[i].y;
            sx += xi; sy += yi;
            sx2 += xi * xi;
            sx3 += xi * xi * xi;
            sx4 += xi * xi * xi * xi;
            sxy += xi * yi;
            sx2y += xi * xi * yi;
            count++;
        }
    }

    if (count < 3) return;

    // Matriks augmented 3x4 untuk persamaan normal
    double M[3][4] = {
        { (double)count, sx,   sx2,  sy   },
        { sx,            sx2,  sx3,  sxy  },
        { sx2,           sx3,  sx4,  sx2y }
    };

    // Eliminasi Gauss
    for (int k = 0; k < 3; k++) {
        double pivot = M[k][k];
        if (fabs(pivot) < 1e-10) continue;

        for (int j = k; j < 4; j++) {
            M[k][j] /= pivot;
        }

        for (int i = 0; i < 3; i++) {
            if (i != k) {
                double factor = M[i][k];
                for (int j = k; j < 4; j++) {
                    M[i][j] -= factor * M[k][j];
                }
            }
        }
    }

    *a0 = M[0][3];
    *a1 = M[1][3];
    *a2 = M[2][3];

    for (int i = 0; i < n; i++) {
        if (data[i].missing_y) {
            double xi = data[i].x;
            y_pred[i] = *a0 + *a1 * xi + *a2 * xi * xi;
            if (y_pred[i] < 0) y_pred[i] = 0; // Pastikan persentase tidak negatif
        } else {
            y_pred[i] = data[i].y; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Regresi polinomial untuk kolom z (Population)
void predictPolynomialZ(DataPoint data[], double z_pred[], int n, double *a0, double *a1, double *a2) {
    double sx = 0, sz = 0, sx2 = 0, sx3 = 0, sx4 = 0, sxz = 0, sx2z = 0;
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_z) {
            double xi = data[i].x, zi = data[i].z;
            sx += xi; sz += zi;
            sx2 += xi * xi;
            sx3 += xi * xi * xi;
            sx4 += xi * xi * xi * xi;
            sxz += xi * zi;
            sx2z += xi * xi * zi;
            count++;
        }
    }

    if (count < 3) return;

    // Matriks augmented 3x4 untuk persamaan normal
    double M[3][4] = {
        { (double)count, sx,   sx2,  sz   },
        { sx,            sx2,  sx3,  sxz  },
        { sx2,           sx3,  sx4,  sx2z }
    };

    // Eliminasi Gauss
    for (int k = 0; k < 3; k++) {
        double pivot = M[k][k];
        if (fabs(pivot) < 1e-10) continue;

        for (int j = k; j < 4; j++) {
            M[k][j] /= pivot;
        }

        for (int i = 0; i < 3; i++) {
            if (i != k) {
                double factor = M[i][k];
                for (int j = k; j < 4; j++) {
                    M[i][j] -= factor * M[k][j];
                }
            }
        }
    }

    *a0 = M[0][3];
    *a1 = M[1][3];
    *a2 = M[2][3];

    for (int i = 0; i < n; i++) {
        if (data[i].missing_z) {
            double xi = data[i].x;
            z_pred[i] = *a0 + *a1 * xi + *a2 * xi * xi;
            if (z_pred[i] < 0) z_pred[i] = 0; // Pastikan populasi tidak negatif
        } else {
            z_pred[i] = data[i].z; // Simpan data asli untuk yang tidak missing
        }
    }
}

// Tulis hasil ke file baru
void writeCSV(const char *filename, DataPoint data[], double y_pred[], double z_pred[], int n) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { perror("Error membuka file untuk menulis"); return; }
    
    fprintf(fp, "Year,Percentage_Internet_User,Population\n");
    
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%.0f,", data[i].x); // Year (tanpa desimal)
        
        if (data[i].missing_y) {
            fprintf(fp, "%.6f,", y_pred[i]); // Predicted Percentage
        } else {
            fprintf(fp, "%.6f,", data[i].y); // Original Percentage
        }
        
        if (data[i].missing_z) {
            fprintf(fp, "%.0f\n", z_pred[i]); // Predicted Population (tanpa desimal)
        } else {
            fprintf(fp, "%.0f\n", data[i].z); // Original Population
        }
    }
    
    fclose(fp);
}

// Plot data asli vs prediksi untuk y (Percentage_Internet_User)
void plotDataY(DataPoint data[], double y_pred[], int n, const char *title) {
    FILE *gp = popen("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\" -p", "w");
    if (!gp) { perror("Error membuka gnuplot"); return; }
    
    fprintf(gp, "set title \"%s\"\n", title);
    fprintf(gp, "set xlabel \"Year\"\n");
    fprintf(gp, "set ylabel \"Percentage Internet User\"\n");
    fprintf(gp, "plot '-' with points pt 7 lc rgb 'blue' title 'Data Asli', \\\n");
    fprintf(gp, "     '-' with points pt 5 lc rgb 'red'  title 'Prediksi'\n");

    // Data asli
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_y) {
            fprintf(gp, "%g %g\n", data[i].x, data[i].y);
        }
    }
    fprintf(gp, "e\n");

    // Data prediksi
    for (int i = 0; i < n; i++) {
        if (data[i].missing_y) {
            fprintf(gp, "%g %g\n", data[i].x, y_pred[i]);
        }
    }
    fprintf(gp, "e\n");

    pclose(gp);
}

// Plot data asli vs prediksi untuk z (Population)
void plotDataZ(DataPoint data[], double z_pred[], int n, const char *title) {
    FILE *gp = popen("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\" -p", "w");
    if (!gp) { perror("Error membuka gnuplot"); return; }
    
    fprintf(gp, "set title \"%s\"\n", title);
    fprintf(gp, "set xlabel \"Year\"\n");
    fprintf(gp, "set ylabel \"Population\"\n");
    fprintf(gp, "plot '-' with points pt 7 lc rgb 'blue' title 'Data Asli', \\\n");
    fprintf(gp, "     '-' with points pt 5 lc rgb 'red'  title 'Prediksi'\n");

    // Data asli
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_z) {
            fprintf(gp, "%g %g\n", data[i].x, data[i].z);
        }
    }
    fprintf(gp, "e\n");

    // Data prediksi
    for (int i = 0; i < n; i++) {
        if (data[i].missing_z) {
            fprintf(gp, "%g %g\n", data[i].x, z_pred[i]);
        }
    }
    fprintf(gp, "e\n");

    pclose(gp);
}

// Fungsi untuk mencari nilai prediksi berdasarkan tahun
double findPredictionForYear(DataPoint data[], double pred[], int n, int year) {
    for (int i = 0; i < n; i++) {
        if (data[i].x == year) {
            return pred[i];
        }
    }
    return -1; // Tahun tidak ditemukan
}

// Fungsi untuk menghitung prediksi untuk tahun di luar data
double calculatePredictionForYear(double a0, double a1, double a2, int year) {
    double result = a0 + a1 * year + a2 * year * year;
    return (result < 0) ? 0 : result; // Pastikan hasilnya tidak negatif
}

// Fungsi untuk menampilkan persamaan polinomial
void displayPolynomialEquation(double a0, double a1, double a2) {
    printf("y = ");
    
    if (fabs(a2) > 1e-10) {
        printf("%g x^2 ", a2);
        if (a1 >= 0) printf("+ ");
    }
    
    if (fabs(a1) > 1e-10) {
        printf("%g x ", a1);
        if (a0 >= 0) printf("+ ");
    }
    
    if (fabs(a0) > 1e-10 || (fabs(a1) < 1e-10 && fabs(a2) < 1e-10)) {
        printf("%g", a0);
    }
    
    printf("\n");
}

int main(int argc, char **argv) {
    const char *input_file = (argc > 1) ? argv[1] : "data.csv";
    DataPoint data[MAX_ROWS];
    
    int n = readCSV(input_file, data);
    if (n <= 0) { printf("Tidak ada data.\n"); return 1; }
    
    printf("Berhasil membaca %d baris data.\n", n);

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