#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_ROWS 1000

typedef struct {
    double x;
    double y;
    int missing_x; // 1 if x is missing
    int missing_y; // 1 if y is missing
    int predicted;  // 1 if value was predicted
} DataPoint;

void writeCSV(const char *filename, DataPoint data[], int n) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { 
        perror("Error writing CSV"); 
        return; 
    }
    fprintf(fp, "Data A,Data B\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%.2f,%.2f\n", data[i].x, data[i].y);
    }
    fclose(fp);
}

void plotGnuplot(DataPoint data[], int n, const char *title) {
    FILE *gp = popen("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\" -p", "w");
    if (!gp) { 
        // Gnuplot not installed
        printf("Warning: Gnuplot not found. Skipping visualization.\n");
        return;
    }
    fprintf(gp, "set title '%s'\n", title);
    fprintf(gp, "plot '-' with points title 'Original', '-' with points title 'Predicted'\n");
    // Original data
    for (int i = 0; i < n; i++) {
        if (!data[i].predicted) fprintf(gp, "%f %f\n", data[i].x, data[i].y);
    }
    fprintf(gp, "e\n");
    // Predicted data
    for (int i = 0; i < n; i++) {
        if (data[i].predicted) fprintf(gp, "%f %f\n", data[i].x, data[i].y);
    }
    fprintf(gp, "e\n");
    pclose(gp);
}

int readCSV(FILE *fp, DataPoint data[]) {
    if (!fp) { 
        return -1; 
    }
    
    char line[1024];
    fgets(line, sizeof(line), fp); // Skip header
    int i = 0;
    while (fgets(line, sizeof(line), fp) && i < MAX_ROWS) {
        line[strcspn(line, "\r\n")] = 0;
        char *x_str = strtok(line, ",");
        char *y_str = strtok(NULL, ",");

        data[i].missing_x = (x_str == NULL || *x_str == '\0') ? 1 : 0;
        data[i].missing_y = (y_str == NULL || *y_str == '\0') ? 1 : 0;
        data[i].predicted = 0; // Initialize as not predicted

        // Safely convert strings to numbers
        if (!data[i].missing_x) {
            char *endptr;
            data[i].x = strtod(x_str, &endptr);
            // Check if conversion failed
            if (endptr == x_str) {
                data[i].missing_x = 1;
                data[i].x = 0;
            }
        } else {
            data[i].x = 0;
        }

        if (!data[i].missing_y) {
            char *endptr;
            data[i].y = strtod(y_str, &endptr);
            // Check if conversion failed
            if (endptr == y_str) {
                data[i].missing_y = 1;
                data[i].y = 0;
            }
        } else {
            data[i].y = 0;
        }
        
        i++;
    }
    return i;
}

// Forward regression: Predict y from x (if x is known)
void linearRegression(DataPoint data[], int n) {
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_x && !data[i].missing_y) {
            sum_x += data[i].x;
            sum_y += data[i].y;
            sum_xy += data[i].x * data[i].y;
            sum_x2 += data[i].x * data[i].x;
            count++;
        }
    }
    if (count < 2) {
        printf("Warning: Not enough data points for linear regression\n");
        return;
    }
    
    double denom = count * sum_x2 - sum_x * sum_x;
    if (fabs(denom) < 1e-10) {
        printf("Warning: Denominator too small for linear regression\n");
        return;
    }
    
    double slope = (count * sum_xy - sum_x * sum_y) / denom;
    double intercept = (sum_y - slope * sum_x) / count;

    printf("Linear regression: y = %.4f * x + %.4f\n", slope, intercept);

    // Predict missing y (if x is known)
    for (int i = 0; i < n; i++) {
        if (data[i].missing_y && !data[i].missing_x) {
            data[i].y = slope * data[i].x + intercept;
            data[i].missing_y = 0; // Mark as not missing anymore
            data[i].predicted = 1; // Mark as predicted
        }
    }
}

// Reverse regression: Predict x from y (if y is known)
void reverseRegression(DataPoint data[], int n) {
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_y2 = 0;
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_x && !data[i].missing_y) {
            sum_x += data[i].x;
            sum_y += data[i].y;
            sum_xy += data[i].x * data[i].y;
            sum_y2 += data[i].y * data[i].y;
            count++;
        }
    }
    if (count < 2) {
        printf("Warning: Not enough data points for reverse regression\n");
        return;
    }
    
    double denom = count * sum_y2 - sum_y * sum_y;
    if (fabs(denom) < 1e-10) {
        printf("Warning: Denominator too small for reverse regression\n");
        return;
    }
    
    double slope = (count * sum_xy - sum_x * sum_y) / denom;
    double intercept = (sum_x - slope * sum_y) / count;

    printf("Reverse regression: x = %.4f * y + %.4f\n", slope, intercept);

    // Predict missing x (if y is known)
    for (int i = 0; i < n; i++) {
        if (data[i].missing_x && !data[i].missing_y) {
            data[i].x = slope * data[i].y + intercept;
            data[i].missing_x = 0; // Mark as not missing anymore
            data[i].predicted = 1; // Mark as predicted
        }
    }
}

// Lagrange interpolation for y (requires known x)
void lagrangeInterpolation(DataPoint data[], int n) {
    DataPoint known[MAX_ROWS];
    int k = 0;
    
    // Copy known data points to a separate array
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_x && !data[i].missing_y) {
            known[k++] = data[i];
        }
    }
    
    if (k < 2) {
        printf("Warning: Not enough data points for Lagrange interpolation\n");
        return;
    }
    
    for (int i = 0; i < n; i++) {
        if (data[i].missing_y && !data[i].missing_x) {
            double result = 0;
            for (int j = 0; j < k; j++) {
                double term = known[j].y;
                for (int m = 0; m < k; m++) {
                    if (m != j) {
                        double divisor = known[j].x - known[m].x;
                        if (fabs(divisor) < 1e-10) {
                            // Skip this term if divisor is too small
                            term = 0;
                            break;
                        }
                        term *= (data[i].x - known[m].x) / divisor;
                    }
                }
                result += term;
            }
            data[i].y = result;
            data[i].missing_y = 0;
            data[i].predicted = 1; // Mark as predicted
        }
    }
}

// Handle rows where both x and y are missing (simple extrapolation)
void extrapolateMissing(DataPoint data[], int n) {
    if (n < 3) {
        printf("Warning: Not enough data points for extrapolation\n");
        return;
    }
    
    double last_x = 0, last_y = 0;
    int last_valid_idx = -1;
    
    // Find the last valid data point first
    for (int i = 0; i < n; i++) {
        if (!data[i].missing_x && !data[i].missing_y) {
            last_x = data[i].x;
            last_y = data[i].y;
            last_valid_idx = i;
        }
    }
    
    if (last_valid_idx < 2) {
        printf("Warning: Not enough valid data points for extrapolation\n");
        return;
    }
    
    for (int i = 0; i < n; i++) {
        if (data[i].missing_x && data[i].missing_y) {
            if (i >= 2 && !data[i-1].missing_x && !data[i-1].missing_y && 
                !data[i-2].missing_x && !data[i-2].missing_y) {
                // Safe to extrapolate based on previous two points
                data[i].x = 2 * data[i-1].x - data[i-2].x; // Linear extrapolation
                data[i].y = 2 * data[i-1].y - data[i-2].y; // Linear extrapolation
            } else {
                // Fallback to simpler method
                data[i].x = last_x + 1;
                data[i].y = last_y;
            }
            data[i].missing_x = 0;
            data[i].missing_y = 0;
            data[i].predicted = 1; // Mark as predicted
        }
    }
}

void printUsage(char* programName) {
    printf("Usage: %s <input_csv_file>\n", programName);
    printf("   or: %s (to read from stdin)\n", programName);
}

int main(int argc, char *argv[]) {
    DataPoint data[MAX_ROWS];
    int n;
    FILE *fp;
    
    // Handle command line arguments
    if (argc > 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    if (argc == 1) {
        // Read from stdin
        fp = stdin;
        printf("Reading from standard input...\n");
    } else {
        // Read from specified file
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("Error opening input file");
            return 1;
        }
        printf("Reading from file: %s\n", argv[1]);
    }
    
    // Read the CSV data
    n = readCSV(fp, data);
    
    if (n <= 0) {
        printf("Error: No data read or file error occurred\n");
        if (fp != stdin) fclose(fp);
        return 1;
    }
    
    if (fp != stdin) fclose(fp);
    
    printf("Read %d data points\n", n);

    // Step 1: Predict missing x using reverse regression
    reverseRegression(data, n);

    // Step 2: Predict missing y using forward regression
    linearRegression(data, n);

    // Step 3: Refine y predictions with Lagrange
    lagrangeInterpolation(data, n);

    // Step 4: Handle fully missing rows (if any)
    extrapolateMissing(data, n);

    // Write results to CSV files
    writeCSV("output_predicted.csv", data, n);
    printf("Predictions saved to output_predicted.csv\n");

    // Plot results
    plotGnuplot(data, n, "Data Prediction Results");

    return 0;
}