package com.scut.lab1;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.net.URI;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.util.GenericOptionsParser;

public class ZipDecompressor {

    public static void main(String[] args) throws Exception {
        Configuration conf = new Configuration();
        String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
        if (otherArgs.length != 2) {
            System.err.println("Usage: ZipDecompressor <zip file> <output directory>");
            System.exit(-1);
        }

        // get zip file
        String zipUri = otherArgs[0];
        String outputDirectory = otherArgs[1];
        if (!zipUri.toLowerCase().endsWith(".zip")) {
            System.err.println("Usage: ZipDecompressor <zip file> <output directory>");
            System.exit(-1);
        }
        FileSystem fs = FileSystem.get(URI.create(zipUri), conf);
        Path zipPath = new Path(zipUri);

        // decompress zip file
        try (
            FSDataInputStream hdfsIs = fs.open(zipPath);
            // ZipInputStream zipIs = new ZipInputStream(new BufferedInputStream(hdfsIs));
            ZipInputStream zipIs = new ZipInputStream(hdfsIs);
            BufferedInputStream bufferedIs = new BufferedInputStream(zipIs);
        ) {
            ZipEntry entry = null;
            while ((entry = zipIs.getNextEntry()) != null) {
                Path entryPath = new Path(outputDirectory + "/" + entry.getName());
                if (entry.isDirectory()) {
                    fs.mkdirs(entryPath);
                    continue;
                }
                try (
                    FSDataOutputStream hdfsOs = fs.create(entryPath);
                    BufferedOutputStream bufferedOs = new BufferedOutputStream(hdfsOs);
                ) {
                    int b;
                    while ((b = bufferedIs.read()) != -1) {
                        bufferedOs.write(b);
                    }
                    bufferedOs.flush();
                }
            }
        }
    }
}

// rm -rf bin; mkdir bin
// javac -classpath `hadoop classpath` -d bin
// src/com/scut/lab1/ZipDecompressor.java
// jar -cvf ZipDecompressor.jar -C bin/ .
// hadoop jar ZipDecompressor.jar com.scut.lab1.ZipDecompressor ./result_txt.zip
// .
// hadoop fs -count ./result_txt
