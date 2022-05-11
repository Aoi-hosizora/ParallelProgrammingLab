package com.scut.lab1;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.CombineTextInputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

public class InvertedIndexBuilder {

    /**
     * Mapper.
     *
     * Input value:  "url；word word word ..."
     * Output key:   "word"
     * Output value: "url，1"
     */
    public static class InvertedIndexMapper extends Mapper<LongWritable, Text, Text, Text> {
        @Override
        public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
            // parse the inputted value
            String textContent = value.toString().trim();
            String[] pair = textContent.split("；");
            if (pair.length < 2) {
                System.err.printf("[InvertedIndexMapper] Warning: invalid value \"%s\"\n", textContent);
                return;
            }

            // get url and words from inputted value
            String url = pair[0].trim();
            String[] words = pair[1].trim().split(" ");
            if (url.isEmpty() || !url.startsWith("http") || words.length == 0) {
                System.err.printf("[InvertedIndexMapper] Warning: invalid value \"%s\"\n", textContent);
                return;
            }

            // build inverted index from word to urls
            for (String word : words) {
                if (word.isEmpty()) {
                    continue;
                }
                word = word.toLowerCase();
                context.write(new Text(word), new Text(url + "，1")); // 注: 全角逗号
            }
        }
    }

    /**
     * Combiner.
     *
     * Input key:    "word"
     * Input values: ["url，sum", "url，sum", "url，sum", ...]
     * Output key:   "word"
     * Output value: "url，sum"
     */
    public static class InvertedIndexCombiner extends Reducer<Text, Text, Text, Text> {
        @Override
        protected void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
            // build <url, sum> map for a single word
            HashMap<String, Integer> map = new HashMap<>();
            for (Text value : values) {
                // parse the current value
                String urlAndSum = value.toString();
                String[] pair = urlAndSum.split("，");
                if (pair.length < 2) {
                    System.err.printf("[InvertedIndexCombiner] Warning: invalid value \"%s\" from key \"%s\"\n", urlAndSum, key.toString());
                    continue;
                }

                // get url and sum from value
                String url = pair[0];
                int sum = 0;
                try {
                    sum = Integer.parseInt(pair[1]);
                } catch (Exception ex) {
                    System.err.printf("[InvertedIndexCombiner] Warning: invalid value \"%s\" from key \"%s\"\n", urlAndSum, key.toString());
                    continue;
                }

                // update sum to map
                if (!map.containsKey(url)) {
                    map.put(url, sum);
                } else {
                    map.put(url, map.get(url) + sum);
                }
            }

            // keep the same types with inputted
            for (Map.Entry<String, Integer> entry : map.entrySet()) {
                String url = entry.getKey();
                int sum = entry.getValue();
                context.write(key, new Text(url + "，" + sum)); // 注: 全角逗号
            }
        }
    }

    /**
     * Reducer.
     *
     * Input key:    "word"
     * Input values: ["url，sum", "url，sum", "url，sum", ...]
     * Output key:   "word"
     * Output value: "url，sum；url，sum；url，sum；..."
     */
    public static class InvertedIndexReducer extends Reducer<Text, Text, Text, Text> {
        @Override
        protected void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
            // combine all <url, sum> for a single word
            StringBuilder sb = new StringBuilder();
            for (Text value : values) {
                sb.append(value.toString() + "；"); // 注: 全角分号
            }
            if (sb.length() == 0) {
                System.err.printf("[InvertedIndexReducer] Warning: key \"%s\" has no value.\n", key.toString());
                return;
            }
            context.write(key, new Text(sb.toString()));
        }
    }

    public static void main(String[] args) throws Exception {
        Configuration conf = new Configuration();
        String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
        if (otherArgs.length != 2) {
            System.err.println("Usage: InvertedIndexBuilder <in> <out>");
            System.exit(-1);
        }

        // create a job and configure metadata
        Job job = Job.getInstance(conf, "InvertedIndexBuilderJob");
        job.setJarByClass(InvertedIndexBuilder.class);
        job.setMapperClass(InvertedIndexMapper.class);
        job.setCombinerClass(InvertedIndexCombiner.class);
        job.setReducerClass(InvertedIndexReducer.class);
        job.setNumReduceTasks(5); // => 1.75 * #nodes, used to increase load balancing and lower the cost of failures
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(Text.class);
        job.setInputFormatClass(CombineTextInputFormat.class);
        CombineTextInputFormat.setMaxInputSplitSize(job, 1024 * 1024 * 32); // => 32MB, used to combine small files

        // setup Input/Output path
        FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
        FileOutputFormat.setOutputPath(job, new Path(otherArgs[1]));

        // run the job
        boolean ok = job.waitForCompletion(true);
        if (!ok) {
            System.err.println("Failed to run the job.");
            System.exit(-1);
        }
    }
}

// rm -rf bin; mkdir bin
// javac -classpath `hadoop classpath` -d bin src/com/scut/lab1/InvertedIndexBuilder.java
// jar -cvf InvertedIndexBuilder.jar -C bin/ .
// hadoop fs -rm -r ./output
// hadoop jar InvertedIndexBuilder.jar com.scut.lab1.InvertedIndexBuilder ./result_txt ./output
