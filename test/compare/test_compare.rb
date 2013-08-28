require "test/unit"

class TestCompare < Test::Unit::TestCase

  def setup
  end


  def teardown
  end


  def compare(a, b)
    out = `compare -metric MAE '#{a}' '#{b}' null: 2>&1`
    out.strip!
    assert_match(/([\d.]+) \(([\d.]+)\)/, out)
    m = out.match(/([\d.]+) \(([\d.]+)\)/)
    return m[1].to_f, m[2].to_f
  end


  Dir.foreach("input") do |file_name|
    next if file_name.index(".") == 0

    define_method("test_" + file_name.gsub(".bmp", "")) do
      input_path = File.join("input", file_name)
      reference_path = File.join("output", file_name.gsub(".bmp", "_ref.jpeg"))
      output_path = File.join("output", file_name.gsub(".bmp", "_out.jpeg"))
      `convert '#{input_path}' '#{reference_path}'`
      assert(File.exists?(reference_path))
      `../convert '#{input_path}' '#{output_path}'`
      assert(File.exists?(output_path))
      reference = compare(input_path, reference_path)
      puts reference
      output = compare(input_path, output_path)
      assert(output[0] < 10000)
      assert((reference[0] - output[0]).abs / reference[0] < 0.1)
      assert((reference[1] - output[1]).abs / reference[1] < 0.1)
    end
  end

end
