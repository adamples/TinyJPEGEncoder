require "test/unit"


class TestTime < Test::Unit::TestCase

  def setup
  end


  def teardown
  end


  def benchmark(command, n = 1)
    avg = 0

    n.times do
      t = Time.now
      `#{command}`
      avg += Time.now - t
    end

    return avg / n
  end


  [ "horizontal.bmp", "vertical.bmp", "random.bmp" ].each do |file_name|
    next if file_name.index(".") == 0

    define_method("test_" + file_name.gsub(".bmp", "")) do
      q = File.read("../../src/tiny_jpeg.h").match(/TJPEG_QUALITY\s+(\d+)/)[1].to_i
      ref = benchmark("convert '#{file_name}' -quality #{q} 'output_ref.jpeg'")
      tiny = benchmark("../convert '#{file_name}' 'output_tiny.jpeg'")
      puts "\n#{file_name}:  ref = #{ref};  tiny = #{tiny}"
      assert(tiny < ref, "TinyJPEG is not ready yet")
      `rm -rf -- 'output_ref.jpeg'`
      `rm -rf -- 'output_tiny.jpeg'`
    end
  end

end
