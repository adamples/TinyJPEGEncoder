require "test/unit"

JPEG_HEADER_LENGTH = 0x152


class TestCompare < Test::Unit::TestCase

  def setup
  end


  def teardown
  end


  def compare(a, b)
    out = `compare -metric MAE '#{a}' '#{b}' null: 2>&1`
    out.strip!
    assert_match(/([\d.]+) \(([\d.]+)\)/, out,
      "Could not determine metric for file #{b}")
    m = out.match(/([\d.]+) \(([\d.]+)\)/)
    return m[1].to_f, m[2].to_f
  end


  def check_markers(path)
    File.open(path, "rb") do |io|
      io.read(JPEG_HEADER_LENGTH)

      while ch = io.read(1)
        if ch == 0xff
          n = io.read(1)
          assert(n == 0x00 || (n == 0xd9 && io.read(1).nil?),
            "Invalid marker found in file '#{path}'")
        end
      end
    end
  end


  Dir.foreach("input") do |file_name|
    next if file_name.index(".") == 0

    define_method("test_" + file_name.gsub(".bmp", "")) do
      input_path = File.join("input", file_name)
      reference_path = File.join("output", file_name.gsub(".bmp", "_ref.jpeg"))
      output_path = File.join("output", file_name.gsub(".bmp", "_out.jpeg"))
      `convert '#{input_path}' -quality 80 '#{reference_path}'`
      assert(File.exists?(reference_path), "File #{reference_path} not created")
      check_markers(reference_path)
      `../convert '#{input_path}' '#{output_path}'`
      assert(File.exists?(output_path), "File #{output_path} not created")
      check_markers(output_path)
      reference = compare(input_path, reference_path)
      output = compare(input_path, output_path)
      puts "\n#{file_name}: reference: #{reference.join('/')};  output: #{output.join('/')}"

      if reference[0].abs < 0.1 || reference[1].abs < 1e-6
      else
        assert(output[0] < 10000, "Output image is too different from input")
        diff = (output[0] - reference[0]) * 100.0 / reference[0]
        assert(diff < 1000 || diff <= 10, "Difference between output and reference JPEG file to high (%0.1f%% > 10%%)" % diff)
        diff = (output[1] - reference[1]) * 100.0 / reference[1]
        assert(diff < 1000 || diff <= 10, "Difference between output and reference JPEG file to high (%0.1f%% > 10%%)" % diff)
      end
    end
  end

end
