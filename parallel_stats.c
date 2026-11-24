/*
 * ============================================================
 * Paralel Istatistik Hesaplayici (OpenMP)
 * ============================================================
 *
 * Bir tam sayi dizisi uzerinde toplam, ortalama ve maksimum
 * istatistiklerini hem seri hem paralel hesaplar ve
 * performans analizi yaparak hizlanmayi raporlar.
 *
 *   Bolum 1: toplam(), ortalama(), maksimum() - seri ve paralel
 *   Bolum 2: 1, 2, 4, 8 thread icin speedup tablosu
 *   Bolum 3: maksimum() icin critical vs reduction
 *
 * Derleme   : gcc -fopenmp -O2 -o parallel_stats parallel_stats.c
 * Calistirma: ./parallel_stats
 * Thread    : OMP_NUM_THREADS=4 ./parallel_stats
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef _OPENMP
  #include <omp.h>
#else
  /* OpenMP yoksa zamanlama fonksiyonlari devreye girer */
  #define omp_get_wtime() 0.0
  #define omp_set_num_threads(x)
#endif

#define N        10000      /* Gorev 1: dogruluk kontrolu icin kucuk dizi */
#define N_BUYUK  10000000   /* Gorev 2: anlamli speedup icin buyuk dizi    */

int veri[N];            /* Gorev 1 dizisi */
int veri_buyuk[N_BUYUK]; /* Gorev 2 speedup olcum dizisi */


/* ─────────────────────────────────────────────────────────────
 * FONKSIYON PROTOTIPLERI
 * ───────────────────────────────────────────────────────────── */
long long toplam_seri    (int dizi[], int n);
long long toplam_paralel (int dizi[], int n);

double    ortalama_seri    (int dizi[], int n);
double    ortalama_paralel (int dizi[], int n);

int       maksimum_seri    (int dizi[], int n);
int       maksimum_paralel (int dizi[], int n);

/* BONUS (Gorev 3): iki farkli senkronizasyon yontemi */
int       maksimum_critical  (int dizi[], int n);
int       maksimum_reduction (int dizi[], int n);


/* ─────────────────────────────────────────────────────────────
 * YARDIMCI: Speedup tablosu satiri yazdir
 * ───────────────────────────────────────────────────────────── */
static void tablo_satiri(int thread_sayisi, double sure, double referans)
{
    double speedup = referans / sure;
    printf("  %6d | %10.6f | %7.2fx\n", thread_sayisi, sure, speedup);
}


/* ════════════════════════════════════════════════════════════
 * ANA FONKSIYON
 * ════════════════════════════════════════════════════════════ */
int main(void)
{
    /* ─────────────────────────────────────────────────────
     * 1. DIZIYI DOLDUR
     *
     * srand(42): Sabit tohum ile her calistirmada ayni dizi
     * uretilir. Seri ve paralel sonuclar karsilastirilebilir,
     * speedup olcumleri tekrarlanabilir olur.
     * rand() % 1000  ->  [0, 999] araliginda tam sayi
     * ───────────────────────────────────────────────────── */
    srand(42);
    for (int i = 0; i < N; i++) {
        veri[i] = rand() % 1000;
    }
    /* Gorev 2 icin buyuk dizi — ayni tohum, tutarli olcum */
    srand(42);
    for (int i = 0; i < N_BUYUK; i++) {
        veri_buyuk[i] = rand() % 1000;
    }

    double t1, t2;


    /* ════════════════════════════════════════════════════
     * GOREV 1: SERI vs PARALEL
     * ════════════════════════════════════════════════════ */
    printf("+==============================================+\n");
    printf("|       GOREV 1 - Seri vs Paralel             |\n");
    printf("+==============================================+\n\n");


    /* ── TOPLAM ────────────────────────────────────────── */
    printf("--- TOPLAM ---\n");

    t1 = omp_get_wtime();
    long long s_toplam = toplam_seri(veri, N);
    t2 = omp_get_wtime();
    printf("Seri    toplam = %lld  (sure: %.6f s)\n", s_toplam, t2 - t1);

    t1 = omp_get_wtime();
    long long p_toplam = toplam_paralel(veri, N);
    t2 = omp_get_wtime();
    printf("Paralel toplam = %lld  (sure: %.6f s)\n", p_toplam, t2 - t1);

    /* Dogrulama: farkli ise race condition var demektir */
    printf("Kontrol        : %s\n\n",
           (s_toplam == p_toplam) ? "ESIT - dogru" : "FARKLI - RACE CONDITION!");


    /* ── ORTALAMA ──────────────────────────────────────── */
    printf("--- ORTALAMA ---\n");

    t1 = omp_get_wtime();
    double s_ort = ortalama_seri(veri, N);
    t2 = omp_get_wtime();
    printf("Seri    ortalama = %.4f  (sure: %.6f s)\n", s_ort, t2 - t1);

    t1 = omp_get_wtime();
    double p_ort = ortalama_paralel(veri, N);
    t2 = omp_get_wtime();
    printf("Paralel ortalama = %.4f  (sure: %.6f s)\n", p_ort, t2 - t1);

    /* Kayan nokta icin kucuk esik kullaniriz, tam esitlik guvenilmez */
    double fark = s_ort - p_ort;
    if (fark < 0.0) fark = -fark;
    printf("Kontrol          : %s\n\n",
           (fark < 0.0001) ? "ESIT - dogru" : "FARKLI - RACE CONDITION!");


    /* ── MAKSIMUM ──────────────────────────────────────── */
    printf("--- MAKSIMUM ---\n");

    t1 = omp_get_wtime();
    int s_maks = maksimum_seri(veri, N);
    t2 = omp_get_wtime();
    printf("Seri    maksimum = %d  (sure: %.6f s)\n", s_maks, t2 - t1);

    t1 = omp_get_wtime();
    int p_maks = maksimum_paralel(veri, N);
    t2 = omp_get_wtime();
    printf("Paralel maksimum = %d  (sure: %.6f s)\n", p_maks, t2 - t1);

    printf("Kontrol          : %s\n\n",
           (s_maks == p_maks) ? "ESIT - dogru" : "FARKLI - RACE CONDITION!");


    /* ════════════════════════════════════════════════════
     * GOREV 2: THREAD SAYISI vs SPEEDUP
     *
     * omp_set_num_threads(k): Bir sonraki paralel bolgede
     * k thread kullan. Referans sure 1 thread ile olculur,
     * speedup = referans_sure / k_thread_suresi formulu.
     * ════════════════════════════════════════════════════ */
    printf("+==============================================+\n");
    printf("|    GOREV 2 - Thread Sayisi vs Speedup       |\n");
    printf("+==============================================+\n\n");

    /* Anlamli speedup icin N_BUYUK=10M elemanli dizi kullanilir */
    omp_set_num_threads(1);
    t1 = omp_get_wtime();
    toplam_paralel(veri_buyuk, N_BUYUK);
    t2 = omp_get_wtime();
    double referans_sure = t2 - t1;

    printf("  Thread | Sure (s)   | Speedup\n");
    printf("  -------|------------|--------\n");

    tablo_satiri(1, referans_sure, referans_sure);

    int thread_listesi[] = {2, 4, 8};
    int kac = 3;

    for (int i = 0; i < kac; i++) {
        int k = thread_listesi[i];
        omp_set_num_threads(k);

        t1 = omp_get_wtime();
        toplam_paralel(veri_buyuk, N_BUYUK);
        t2 = omp_get_wtime();

        tablo_satiri(k, t2 - t1, referans_sure);
    }

    printf("\n  NOT: Speedup olcumu N=%d elemanli dizi ile yapildi.\n", N_BUYUK);
    printf("       N=10000 kucuk dizide thread baslatma maliyeti\n");
    printf("       hesaplama suresini gecer, speedup gorulmez.\n\n");


    /* ════════════════════════════════════════════════════
     * GOREV 3 (BONUS): critical vs reduction KARSILASTIRMASI
     *
     *   critical  : Her iterasyonda kilit alinir, diger
     *               thread'ler bekler -> yavas.
     *   reduction : Her thread kendi yerel maksimumunu
     *               hesaplar, sonda birlestirilir -> hizli.
     * ════════════════════════════════════════════════════ */
    printf("+==============================================+\n");
    printf("|  GOREV 3 (BONUS) - critical vs reduction    |\n");
    printf("+==============================================+\n\n");

    /* Gorev 3 de N_BUYUK ile olculur; N=10000 cok kucuk */
    omp_set_num_threads(4);

    t1 = omp_get_wtime();
    int maks_critical = maksimum_critical(veri_buyuk, N_BUYUK);
    t2 = omp_get_wtime();
    double sure_critical = t2 - t1;
    printf("critical  maksimum = %d  (sure: %.6f s)\n", maks_critical, sure_critical);

    t1 = omp_get_wtime();
    int maks_reduction = maksimum_reduction(veri_buyuk, N_BUYUK);
    t2 = omp_get_wtime();
    double sure_reduction = t2 - t1;
    printf("reduction maksimum = %d  (sure: %.6f s)\n", maks_reduction, sure_reduction);

    printf("Sonuc kontrolu    : %s\n",
           (maks_critical == maks_reduction) ? "ESIT - dogru" : "FARKLI - HATA!");

    if (sure_critical > 0.0 && sure_reduction > 0.0) {
        if (sure_critical >= sure_reduction) {
            printf("reduction, critical'dan %.1fx daha hizli\n\n",
                   sure_critical / sure_reduction);
        } else {
            printf("critical, reduction'dan %.1fx daha hizli (beklenmez)\n\n",
                   sure_reduction / sure_critical);
        }
    } else {
        printf("(Sure farki cok kucuk - N buyutun)\n\n");
    }

    /*
     * ANALIZ - critical neden yavas, reduction neden hizli?
     *
     * critical: Her iterasyonda sadece bir thread maksimum
     *   guncellemesi yapabilir. Diger thread'ler kilit bekler.
     *   N iterasyonun cogu siralasir -> paralel kazanc azalir.
     *
     * reduction(max:m): Her thread kendi yerel maksimumunu
     *   hicbir kilit olmadan hesaplar. Dongu tamamen paralel.
     *   Sonda OpenMP yalnizca (thread_sayisi - 1) karsilastirma
     *   yapar -> hizlanma dogrusal olur.
     *
     * Kural: Sayisal birlestirme (toplam, min, max) icin
     * her zaman reduction kullan; critical veya atomic degil.
     */

    return 0;
}


/* ════════════════════════════════════════════════════════════
 * FONKSIYON TANIMLARI
 * ════════════════════════════════════════════════════════════ */


/* ─────────────────────────────────────────────────────────────
 * TOPLAM - SERI: tek thread, referans davranis
 * ───────────────────────────────────────────────────────────── */
long long toplam_seri(int dizi[], int n)
{
    long long t = 0;
    for (int i = 0; i < n; i++) t += dizi[i];
    return t;
}


/* ─────────────────────────────────────────────────────────────
 * TOPLAM - PARALEL
 *
 * reduction(+:t): Her thread kendi yerel t kopyasini 0'dan
 * baslatir. Dongu bitince OpenMP hepsini toplayip t'ye yazar.
 * shared(t) + t+=dizi[i] race condition olusturur; reduction yapmaz.
 * ───────────────────────────────────────────────────────────── */
long long toplam_paralel(int dizi[], int n)
{
    long long t = 0;

    #pragma omp parallel for reduction(+:t)
    for (int i = 0; i < n; i++) {
        t += dizi[i];
    }

    return t;
}


/* ─────────────────────────────────────────────────────────────
 * ORTALAMA - SERI
 * ───────────────────────────────────────────────────────────── */
double ortalama_seri(int dizi[], int n)
{
    long long t = 0;
    for (int i = 0; i < n; i++) t += dizi[i];
    return (double)t / n;  /* Tam sayi bolmesi olmassin diye cast */
}


/* ─────────────────────────────────────────────────────────────
 * ORTALAMA - PARALEL
 * Toplami reduction ile paralel hesapla, bolme tek thread'de.
 * ───────────────────────────────────────────────────────────── */
double ortalama_paralel(int dizi[], int n)
{
    long long t = 0;

    #pragma omp parallel for reduction(+:t)
    for (int i = 0; i < n; i++) {
        t += dizi[i];
    }

    return (double)t / n;
}


/* ─────────────────────────────────────────────────────────────
 * MAKSIMUM - SERI: klasik dogrusal tarama
 * ───────────────────────────────────────────────────────────── */
int maksimum_seri(int dizi[], int n)
{
    int m = dizi[0];
    for (int i = 1; i < n; i++) {
        if (dizi[i] > m) m = dizi[i];
    }
    return m;
}


/* ─────────────────────────────────────────────────────────────
 * MAKSIMUM - PARALEL (reduction)
 *
 * reduction(max:m): OpenMP her thread icin m'nin yerel kopyasini
 * dizi[0] ile baslatir. Dongu sonunda tum yerellerin maxi alinir.
 * OpenMP 3.1+ gerektirir (MinGW-w64 GCC 4.7+ destekler).
 * ───────────────────────────────────────────────────────────── */
int maksimum_paralel(int dizi[], int n)
{
    int m = dizi[0];

    #pragma omp parallel for reduction(max:m)
    for (int i = 1; i < n; i++) {
        if (dizi[i] > m) m = dizi[i];
    }

    return m;
}


/* ─────────────────────────────────────────────────────────────
 * MAKSIMUM - CRITICAL (Gorev 3 - Bonus)
 *
 * #pragma omp critical: Bloga ayni anda tek thread girer.
 * Diger thread'ler kilit icin bekler -> N iterasyon boyunca
 * guncelleme siralanir -> paralel kazanc duser.
 * ───────────────────────────────────────────────────────────── */
int maksimum_critical(int dizi[], int n)
{
    int m = dizi[0];

    #pragma omp parallel for
    for (int i = 1; i < n; i++) {
        /* Kilit: okuma ve yazma arasina baska thread giremez */
        #pragma omp critical
        {
            if (dizi[i] > m) m = dizi[i];
        }
    }

    return m;
}


/* ─────────────────────────────────────────────────────────────
 * MAKSIMUM - REDUCTION (Gorev 3 - Bonus)
 *
 * Hicbir kilit yok: her thread kendi yerel maksimumunu bulur.
 * Dongu tamamen paralel, sonunda tek bir birlesim asamasi var.
 * ───────────────────────────────────────────────────────────── */
int maksimum_reduction(int dizi[], int n)
{
    int m = dizi[0];

    #pragma omp parallel for reduction(max:m)
    for (int i = 1; i < n; i++) {
        if (dizi[i] > m) m = dizi[i];
    }

    return m;
}
